/***********************************************************************/
/* COMMSET1.C - SET commands A-N                                       */
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

static char RCSid[] = "$Id: commset1.c,v 1.18 2002/08/13 09:11:47 mark Exp $";

#include <the.h>
#include <proto.h>

the_header_mapping thm[] =
{
   {"NUMBER",                6,  HEADER_NUMBER      },
   {"COMMENT",               7,  HEADER_COMMENT     },
   {"STRING",                6,  HEADER_STRING      },
   {"KEYWORD",               7,  HEADER_KEYWORD     },
   {"FUNCTION",              8,  HEADER_FUNCTION    },
   {"HEADER",                6,  HEADER_HEADER      },
   {"LABEL",                 5,  HEADER_LABEL       },
   {"MATCH",                 5,  HEADER_MATCH       },
   {"COLUMN",                6,  HEADER_COLUMN      },
   {"POSTCOMPARE",          11,  HEADER_POSTCOMPARE },
   {"MARKUP",                6,  HEADER_MARKUP      },
   {"*",                     1,  HEADER_ALL         }, /* this should be last */
   {NULL,                    0,  0                  },
};

/*#define DEBUG 1*/

/***********************************************************************/
#ifdef HAVE_PROTO
static short set_active_colour( short area )
#else
static short set_active_colour( area )
short area;
#endif
/***********************************************************************/
{
   int i;
   COLOUR_ATTR attr;
   chtype ch=0L,nondisp_attr=0L;

   memcpy( &attr, CURRENT_FILE->attr+area, sizeof(COLOUR_ATTR) );
   /*
    * Special handling required for NONDISP...
    */
   if (area == ATTR_NONDISP )
   {
      nondisp_attr = set_colour( &attr );
      for ( i = 0 ; i < 256; i++ )
      {
         if (etmode_flag[i])
         {
            ch = etmode_table[i] & A_CHARTEXT;
            etmode_table[i] = ch | nondisp_attr;
         }
      }
   }
   /*
    * If we haven't started curses (in profile first time) exit now...
    */
   if (!curses_started)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
#if defined(USE_XCURSES) && PDC_BUILD >= 2501
   /*
    * For the special BOUNDMARK colour, set the curses global colour
    * to the foreground and return
    */
   if ( area == ATTR_BOUNDMARK)
   {
      PDC_set_line_color(FOREFROMPAIR(attr.pair));
      build_screen(current_screen);
      display_screen(current_screen);
      TRACE_RETURN();
      return(RC_OK);
   }
#endif
   /*
    * Update the appropriate window with the new colour combination...
    */
   switch (valid_areas[area].area_window)
   {
      case WINDOW_FILEAREA:
         if (area == ATTR_FILEAREA)
            wattrset(CURRENT_WINDOW_FILEAREA,set_colour(CURRENT_FILE->attr+area));
         build_screen(current_screen);
         display_screen(current_screen);
#if defined(USE_XCURSES) && PDC_BUILD >= 2501
         if ( area == ATTR_BOUNDMARK)
         {
            redraw_window(CURRENT_WINDOW_FILEAREA);
            touchwin(CURRENT_WINDOW_FILEAREA);
            wnoutrefresh(CURRENT_WINDOW_FILEAREA);
         }
#endif
         break;
      case WINDOW_PREFIX:
         if (CURRENT_WINDOW_PREFIX != NULL)
         {
            wattrset(CURRENT_WINDOW_PREFIX,set_colour(CURRENT_FILE->attr+area));
            build_screen(current_screen);
            display_screen(current_screen);
         }
         break;
      case WINDOW_COMMAND:
         if (CURRENT_WINDOW_COMMAND != NULL)
         {
            wattrset(CURRENT_WINDOW_COMMAND,set_colour(CURRENT_FILE->attr+area));
            redraw_window(CURRENT_WINDOW_COMMAND);
            touchwin(CURRENT_WINDOW_COMMAND);
            wnoutrefresh(CURRENT_WINDOW_COMMAND);
         }
         break;
      case WINDOW_ARROW:
         if (CURRENT_WINDOW_ARROW != NULL)
         {
            wattrset(CURRENT_WINDOW_ARROW,set_colour(CURRENT_FILE->attr+area));
            redraw_window(CURRENT_WINDOW_ARROW);
            touchwin(CURRENT_WINDOW_ARROW);
            wnoutrefresh(CURRENT_WINDOW_ARROW);
         }
         break;
      case WINDOW_IDLINE:
         if (CURRENT_WINDOW_IDLINE != NULL)
         {
            wattrset(CURRENT_WINDOW_IDLINE,set_colour(CURRENT_FILE->attr+area));
            redraw_window(CURRENT_WINDOW_IDLINE);
            touchwin(CURRENT_WINDOW_IDLINE);
            wnoutrefresh(CURRENT_WINDOW_IDLINE);
         }
         break;
      case WINDOW_STATAREA:
         if (statarea != NULL)
         {
            wattrset(statarea,set_colour(CURRENT_FILE->attr+area));
            redraw_window(statarea);
            touchwin(statarea);
            wnoutrefresh(statarea);
         }
         break;
      case WINDOW_FILETABS:
         if (filetabs != NULL)
         {
            wattrset(filetabs,set_colour(CURRENT_FILE->attr+area));
            redraw_window(filetabs);
            touchwin(filetabs);
            wnoutrefresh(filetabs);
         }
         break;
      case WINDOW_DIVIDER:
         if (divider != (WINDOW *)NULL)
         {
            wattrset(divider,set_colour(CURRENT_FILE->attr+area));
            if (display_screens > 1
            &&  !horizontal)
            {
               draw_divider();
               touchwin(divider);
               wnoutrefresh(divider);
            }
         }
         break;
      case WINDOW_SLK:
#if defined(HAVE_SLK_INIT)
         if (SLKx)
         {
#if defined(HAVE_SLK_ATTRSET)
            slk_attrset(set_colour(CURRENT_FILE->attr+area));
#else
            display_error(61,(CHARTYPE *)"slk_attrset not in curses library",FALSE);
#endif
            slk_touch();
            slk_noutrefresh();
         }
#endif
         break;
      default:
         break;
   }
   TRACE_RETURN();
   return(RC_OK);
}

#ifdef HAVE_PROTO
short Birthday(CHARTYPE *params)
#else
short Birthday(params)
CHARTYPE *params;
#endif
{
 short rc=RC_OK;
 rc = execute_set_on_off(params,&BIRTHDAYx);
 return(rc);
}

/*man-start*********************************************************************


========================================================================
SET COMMAND REFERENCE
========================================================================
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     set alt - change alteration counts

SYNTAX
     [SET] ALT [n] [m]

DESCRIPTION
     The SET ALT command allows the user to change the alteration counts.
     This command is usually called from within a macro.

     The first number; 'n' sets the number of changes since the last
     AUTOSAVE was issued.

     The second number; 'm' sets the number of changes since the last
     SAVE or SSAVE command was issued.

     All options can be specified as the current EQUIVCHAR to retain the
     existing value.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     OFF

SEE ALSO
     <SET EQUIVCHAR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Alt(CHARTYPE *params)
#else
short Alt(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define ALT_PARAMS  2
   CHARTYPE strip[ALT_PARAMS];
   CHARTYPE *word[ALT_PARAMS+1];
   unsigned short num_params=0;
   unsigned short autosave_alt=CURRENT_FILE->autosave_alt;
   unsigned short save_alt=CURRENT_FILE->save_alt;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Alt");
   strip[0]=STRIP_BOTH;
   num_params = param_split(params,word,ALT_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
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
   if ( equal( word[0], EQUIVCHARstr, 1 ) )
      autosave_alt = CURRENT_FILE->autosave_alt;
   else
   {
      if (!valid_positive_integer(word[0]))
      {
         display_error(1,word[0],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      autosave_alt = atoi((DEFCHAR *)word[0]);
   }
   if (num_params == 2)
   {
      if ( equal( word[1], EQUIVCHARstr, 1 ) )
         save_alt = CURRENT_FILE->save_alt;
      else
      {
         if (!valid_positive_integer(word[1]))
         {
            display_error(1,word[1],FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
         }
         save_alt = atoi((DEFCHAR *)word[1]);
      }
   }

   CURRENT_FILE->autosave_alt = autosave_alt;
   CURRENT_FILE->save_alt = save_alt;

   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set arbchar - set arbitrary character(s) for targets

SYNTAX
     [SET] ARBchar ON|OFF [char1] [char2]

DESCRIPTION
     Set the character to use as an 'arbitrary character' in string
     targets. The first arbitrary character matches a group of zero
     or more characters, the second will match exactly one character.

     All options can be specified as the current EQUIVCHAR to retain the
     existing value.

COMPATIBILITY
     XEDIT: Compatible.
            Single arbitrary character not supported.
     KEDIT: Compatible.

DEFAULT
     Off $ ?

SEE ALSO
     <SET EQUIVCHAR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Arbchar(CHARTYPE *params)
#else
short Arbchar(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define ARB_PARAMS  4
   CHARTYPE *word[ARB_PARAMS+1];
   CHARTYPE strip[ARB_PARAMS];
   unsigned short num_params=0;
   short rc=RC_INVALID_OPERAND;
   bool arbsts=CURRENT_VIEW->arbchar_status;
   CHARTYPE arbchr_single=CURRENT_VIEW->arbchar_single;
   CHARTYPE arbchr_multiple=CURRENT_VIEW->arbchar_multiple;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Arbchar");
   /*
    * Validate the parameters that have been supplied.
    */
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   num_params = param_split(params,word,ARB_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   switch(num_params)
   {
/*---------------------------------------------------------------------*/
/* No parameters, error.                                               */
/*---------------------------------------------------------------------*/
      case 0:
         display_error(3,(CHARTYPE *)"",FALSE);
         break;
/*---------------------------------------------------------------------*/
/* 1 or 2 parameters, validate them...                                 */
/*---------------------------------------------------------------------*/
      case 1:
         if ( equal( word[0], EQUIVCHARstr, 1 ) )
            arbsts = arbsts;
         else
            rc = execute_set_on_off(word[0],&arbsts);
         break;
      case 2:
      case 3:
         if ( equal( word[0], EQUIVCHARstr, 1 ) )
            arbsts = arbsts;
         else
         {
            rc = execute_set_on_off(word[0],&arbsts);
            if (rc != RC_OK)
              break;
         }
         rc = RC_INVALID_OPERAND;
/*---------------------------------------------------------------------*/
/* For 2 parameters, check that a single character has been supplied...*/
/*---------------------------------------------------------------------*/
         if ( equal( word[1], EQUIVCHARstr, 1 ) )
            arbchr_multiple = arbchr_multiple;
         else
         {
            if (strlen((DEFCHAR *)word[1]) != 1)
            {
               display_error(1,word[1],FALSE);
               break;
            }
            arbchr_multiple = word[1][0];
         }
         rc = RC_OK;
/*---------------------------------------------------------------------*/
/* For 2 parameters, don't check any more.                             */
/*---------------------------------------------------------------------*/
         if (num_params == 2)
            break;
         rc = RC_INVALID_OPERAND;
/*---------------------------------------------------------------------*/
/* For 3 parameters, check that a single character has been supplied...*/
/*---------------------------------------------------------------------*/
         if ( equal( word[2], EQUIVCHARstr, 1 ) )
            arbchr_single = arbchr_single;
         else
         {
            if (strlen((DEFCHAR *)word[2]) != 1)
            {
               display_error(1,word[2],FALSE);
               break;
            }
            arbchr_single = word[2][0];
         }
         rc = RC_OK;
         break;
/*---------------------------------------------------------------------*/
/* Too many parameters...                                              */
/*---------------------------------------------------------------------*/
      default:
         display_error(2,(CHARTYPE *)"",FALSE);
         break;
   }
/*---------------------------------------------------------------------*/
/* If valid parameters, change the settings...                         */
/*---------------------------------------------------------------------*/
   if (rc == RC_OK)
   {
      CURRENT_VIEW->arbchar_single = arbchr_single;
      CURRENT_VIEW->arbchar_multiple = arbchr_multiple;
      CURRENT_VIEW->arbchar_status = arbsts;
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set autocolor - specifies which parser to use for syntax highlighting

SYNTAX
     [SET] AUTOCOLOR mask parser [MAGIC]

DESCRIPTION
     The SET AUTOCOLOR command allows the user to specify which syntax
     highlighting <parser> is to be used for which file masks.

     The 'parser' argument specifies a syntax highlighting <parser> that
     already exists, either as a default <parser>, or added by the user
     with <SET PARSER>.  The special parser name of '*NULL' can be
     specified; this will effectively remove the association between
     the <parser> and the file mask.

     The 'mask' argument specifies the file mask (or <magic number>) to
     associate with the specified parser.  The 'mask' can be any valid
     file mask for the operating system. eg *.c fred.* joe.?

     If the 'magic' option is specified, the 'mask' argument refers to
     the last element of the <magic number> that is specified in the
     first line of a Unix shell script comment. eg if the first line of
     a shell script contains:
     #!/usr/local/bin/rexx
     then the file mask argument would be specified as "rexx".

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Similar. KEDIT does not have MAGIC option.

DEFAULT
     See <QUERY> AUTOCOLOR

SEE ALSO
     <SET COLORING>, <SET ECOLOUR>, <SET PARSER>

STATUS
     Complete.
**man-end**********************************************************************/
/*man-start*********************************************************************
COMMAND
     set autocolour - specifies which parser to use for syntax highlighting

SYNTAX
     [SET] AUTOCOLOR mask parser [MAGIC]

DESCRIPTION
     The SET AUTOCOLOUR command is a synonym for the <SET AUTOCOLOR> command.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Similar. KEDIT does not have MAGIC option.

DEFAULT
     See <QUERY> AUTOCOLOR

SEE ALSO
     <SET AUTOCOLOR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Autocolour(CHARTYPE *params)
#else
short Autocolour(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define AUCO_PARAMS  3
   CHARTYPE *word[AUCO_PARAMS+1];
   CHARTYPE strip[AUCO_PARAMS];
   unsigned short num_params=0;
   short rc=RC_OK;
   PARSER_DETAILS *parser=NULL;
   PARSER_MAPPING *mapping=NULL,*curr=NULL,*tmp_mapping;
   CHARTYPE *filemask=NULL,*magic_number=NULL;
   VIEW_DETAILS *curr_view=vd_first;
   bool redisplay_current=FALSE,redisplay_other=FALSE;
   int i,change=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Autocolour");
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied.                    */
/*---------------------------------------------------------------------*/
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   num_params = param_split(params,word,AUCO_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 2)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   filemask = word[0];
   if (num_params == 3)
   {
      if (equal((CHARTYPE *)"magic",word[2],5))
      {
         magic_number = word[0];
         filemask = NULL;
      }
      else
      {
         display_error(1,word[2],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
   }
   /*
    * Find a parser equal to the first parameter...
    */
   if (!equal((CHARTYPE *)"*null",word[1],5))
   {
      parser = parserll_find(first_parser,word[1]);
      if (parser == NULL)
      {
         display_error(199,word[1],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
   }
   /*
    * Now check if we already have a mapping for the mask/magic number
    */
   mapping = mappingll_find(first_parser_mapping,filemask,magic_number);
   if (mapping)
      curr = mapping;
   /*
    * Add the new mapping if it is a "real" parser.
    */
   if (parser)
   {
      curr = last_parser_mapping = mappingll_add(first_parser_mapping,last_parser_mapping,sizeof(PARSER_MAPPING));
      if (first_parser_mapping == NULL)
         first_parser_mapping = curr;
      if (filemask)
      {
         curr->filemask = (CHARTYPE *)(*the_malloc)(1+strlen((DEFCHAR *)filemask)*sizeof(CHARTYPE));
         if (curr->filemask == NULL)
         {
            display_error(30,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_OUT_OF_MEMORY);
         }
         strcpy((DEFCHAR *)curr->filemask,(DEFCHAR *)filemask);
      }
      if (magic_number)
      {
         curr->magic_number = (CHARTYPE *)(*the_malloc)(1+strlen((DEFCHAR *)magic_number)*sizeof(CHARTYPE));
         if (curr->magic_number == NULL)
         {
            display_error(30,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_OUT_OF_MEMORY);
         }
         strcpy((DEFCHAR *)curr->magic_number,(DEFCHAR *)magic_number);
         curr->magic_number_length = strlen((DEFCHAR *)magic_number);
      }
      curr->parser = parser;
   }
   /*
    * Check all files in the ring and apply the new mapping. If the current
    * file or the file in the other screen now match the new mapping,
    * redisplay them.
    */
   for (i=0;i<number_of_files;)
   {
      if (curr
      &&  find_parser_mapping(curr_view->file_for_view,curr))
      {
         curr_view->file_for_view->parser = parser;
         if (curr_view->file_for_view == SCREEN_FILE(current_screen))
            redisplay_current = TRUE;
         if (display_screens > 1
         &&  curr_view->file_for_view == SCREEN_FILE(other_screen))
            redisplay_other = TRUE;
      }
      curr_view = curr_view->next;
      if (curr_view == NULL)
         break;
   }
   /*
    * Now delete the old mapping if we found one earlier...
    */
   if (mapping)
   {
      mappingll_del(&first_parser_mapping,&last_parser_mapping,mapping,DIRECTION_FORWARD);
      change--;
   }
   if (parser)
   {
      change++;
   }
   if (rexx_support)
   {
      if (change > 0)
      {
         CHARTYPE tmp[20];
         /*
          * As this is a new mapping, then register another implied extract
          * function for the number of mappings we now have.
          */
         for(i=0,tmp_mapping=first_parser_mapping;tmp_mapping!=NULL;tmp_mapping=tmp_mapping->next,i++);
         sprintf((DEFCHAR *)tmp,"autocolour.%d",i);
         MyRexxRegisterFunctionExe(tmp);
         sprintf((DEFCHAR *)tmp,"autocolor.%d",i);
         MyRexxRegisterFunctionExe(tmp);
      }
      if (change < 0)
      {
         CHARTYPE tmp[20];
         /*
          * As this is a removal of a mapping, then deregister the implied extract
          * function for the number of mappings we had before.
          */
         for(i=0,tmp_mapping=first_parser_mapping;tmp_mapping!=NULL;tmp_mapping=tmp_mapping->next,i++);
         sprintf((DEFCHAR *)tmp,"autocolour.%d",i-1);
         MyRexxDeregisterFunction(tmp);
         sprintf((DEFCHAR *)tmp,"autocolor.%d",i-1);
         MyRexxDeregisterFunction(tmp);
      }
   }

 if (redisplay_other)
    display_screen(other_screen);
 if (redisplay_current)
    display_screen(current_screen);

 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set autosave - set autosave period

SYNTAX
     [SET] AUtosave n|OFF

DESCRIPTION
     The SET AUTOSAVE command sets the interval between automatic saves
     of the file, or turns it off altogether.  The interval 'n' refers
     to the number of alterations made to the file.  Hence a value of
     10 for 'n' would result in the file being automatically saved after
     each 10 alterations have been made to the file.

     It is not possible to set AUTOSAVE for 'psuedo' files such as the
     directory listing 'file', Rexx output 'file' and the key definitions
     'file'

COMPATIBILITY
     XEDIT: Does not support [mode] option.
     KEDIT: Compatible.

DEFAULT
     OFF

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Autosave(CHARTYPE *params)
#else
short Autosave(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define AUS_PARAMS  1
 CHARTYPE strip[AUS_PARAMS];
 CHARTYPE *word[AUS_PARAMS+1];
 unsigned short num_params=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Autosave");
 strip[0]=STRIP_BOTH;
 num_params = param_split(params,word,AUS_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params == 0)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 if (num_params != 1)
   {
    display_error(2,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 if (equal((CHARTYPE *)"off",word[0],3))
   {
    CURRENT_FILE->autosave = 0;
    TRACE_RETURN();
    return(RC_OK);
   }
 if (!valid_positive_integer(word[0]))
   {
    display_error(4,(CHARTYPE *)word[0],FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 CURRENT_FILE->autosave = (CHARTYPE)atoi((DEFCHAR *)word[0]);
 TRACE_RETURN();
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set autoscroll - set rate of automatic horizontal scrolling

SYNTAX
     [SET] AUTOSCroll n|OFF|Half

DESCRIPTION
     The SET AUTOSCROLL allows the user to set the rate at which automatic
     horizontal scrolling occurs.

     When the cursor reaches the last (or first) column of the <filearea>
     the <filearea> can automatically scroll if AUTOSCROLL is not 'OFF' and
     a <CURSOR> RIGHT or <CURSOR> LEFT command is issued.
     How many columns are scrolled is determined by the setting of AUTOSCROLL.

     If AUTOSCROLL is set to 'HALF', then half the number of columns in the
     <filearea> window are scrolled.  Any other value will result in that
     many columns scrolled, or the full width of the <filearea> window if
     the set number of columns is larger.

     Autoscrolling does not occur if the key pressed is assigned to
     <CURSOR> SCREEN LEFT or RIGHT, which is the case if <SET COMPAT> XEDIT
     key defintions are active.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     HALF

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Autoscroll(CHARTYPE *params)
#else
short Autoscroll(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define AUL_PARAMS  1
 CHARTYPE strip[AUL_PARAMS];
 CHARTYPE *word[AUL_PARAMS+1];
 unsigned short num_params=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Autoscroll");
 strip[0]=STRIP_BOTH;
 num_params = param_split(params,word,AUL_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params == 0)
 {
    display_error(3,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
 }
 if (num_params != 1)
 {
    display_error(2,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
 }
 if (equal((CHARTYPE *)"off",word[0],3))
 {
    CURRENT_VIEW->autoscroll = 0;
    TRACE_RETURN();
    return(RC_OK);
 }
 if (equal((CHARTYPE *)"half",word[0],1))
 {
    CURRENT_VIEW->autoscroll = (-1);
    TRACE_RETURN();
    return(RC_OK);
 }
 if (!valid_positive_integer(word[0]))
 {
    display_error(4,(CHARTYPE *)word[0],FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
 }
 CURRENT_VIEW->autoscroll = (CHARTYPE)atol((DEFCHAR *)word[0]);
 TRACE_RETURN();
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set backup - indicate if a backup copy of the file is to be kept

SYNTAX
     [SET] BACKup OFF|TEMP|KEEP|ON|INPLACE

DESCRIPTION
     The SET BACKUP command allows the user to determine if a backup copy
     of the original file is to be kept when the file being edited is
     saved or filed.

     'KEEP' and 'ON' options are the same. 'ON' is
     kept for compatability with previous versions of THE.

     With 'OFF', the file being written to disk will replace an
     existing file. There is a chance that you will end up with neither
     the old version of the file or the new one if problems occur
     while the file is being written.

     With 'TEMP' or 'KEEP' options, the file being written is first
     renamed to the filename with a .bak extension. The file in memory
     is then written to disk. If 'TEMP' is in effect, the backup
     file is then deleted.

     With 'INPLACE', the file being written is first copied to a file
     with a .bak extension. The file in memory is then written to disk
     in place of the original.  This option ensures that all operating
     system file attributes are retained.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     KEEP

SEE ALSO
     <FILE>, <FFILE>, <SAVE>, <SSAVE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Backup(CHARTYPE *params)
#else
short Backup(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 short backup_type=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Backup");
 if (equal((CHARTYPE *)"off",params,3))
    backup_type = BACKUP_OFF;
 if (equal((CHARTYPE *)"on",params,2))
    backup_type = BACKUP_ON;
 if (equal((CHARTYPE *)"keep",params,4))
    backup_type = BACKUP_KEEP;
 if (equal((CHARTYPE *)"temp",params,4))
    backup_type = BACKUP_TEMP;
 if (equal((CHARTYPE *)"inplace",params,2))
    backup_type = BACKUP_INPLACE;
 if (backup_type == 0)
   {
    display_error(1,params,FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 CURRENT_FILE->backup = backup_type;
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set beep - turn on or off the audible alarm when displaying errors

SYNTAX
     [SET] BEEP ON|OFF

DESCRIPTION
     The SET BEEP command allows the user to determine if an audible
     alarm is sounded when an error is displayed.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     OFF

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short BeepSound(CHARTYPE *params)
#else
short BeepSound(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:BeepSound");
 rc = execute_set_on_off(params,&BEEPx);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set boundmark - set bounds marker display

SYNTAX
     [SET] BOUNDMARK OFF|Zone|TRunc|MARgins|TABs|Verify

DESCRIPTION
     The BOUNDMARK command indicates if boundary markers are to be
     displayed and if so, where. Boundary markers are vertical lines
     drawn before or after certain columns within the <filearea>.
     This command only has a visible effect on GUI platforms, currently
     only the X11 port.

     'OFF' turns off the display of boundary markers.

     'ZONE' turns on the display of boundary markers, before the zone
     start column and after the zone end column.

     'TRUNC' turns on the display of boundary markers, after the
     truncation column. Not supported.

     'MARGINS' turns on the display of boundary markers, before the left
     margin and after the right margin.

     'TABS' turns on the display of boundary markers, before each tab
     column.

     'VERIFY' turns on the display of boundary markers, before each verify
     column. Not supported.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible, but no support for TRUNC or VERIFY option.

DEFAULT
     Zone

STATUS
     Incomplete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Boundmark(CHARTYPE *params)
#else
short Boundmark(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define BND_PARAMS  2
   CHARTYPE save_boundmark=CURRENT_VIEW->boundmark;
   CHARTYPE *word[BND_PARAMS+1];
   CHARTYPE strip[BND_PARAMS];
   short num_params=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Boundmark");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   num_params = param_split(params,word,BND_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if ( num_params == 0 )
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if ( num_params > 1 )
   {
      display_error(2,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Validate the first and only parameter
    */
   if (equal((CHARTYPE *)"zone",word[0],1))
      CURRENT_VIEW->boundmark = BOUNDMARK_ZONE;
   else if (equal((CHARTYPE *)"trunc",word[0],2))
      CURRENT_VIEW->boundmark = BOUNDMARK_TRUNC;
   else if (equal((CHARTYPE *)"margins",word[0],2))
      CURRENT_VIEW->boundmark = BOUNDMARK_MARGINS;
   else if (equal((CHARTYPE *)"tabs",word[0],3))
      CURRENT_VIEW->boundmark = BOUNDMARK_TABS;
   else if (equal((CHARTYPE *)"verify",word[0],1))
      CURRENT_VIEW->boundmark = BOUNDMARK_VERIFY;
   else
   {
      display_error(1,(CHARTYPE *)word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If the value has changed, display the screen
    */
   if ( CURRENT_VIEW->boundmark != save_boundmark )
   {
      build_screen(current_screen);
      display_screen(current_screen);
   }

   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set case - set case sensitivity parameters

SYNTAX
     [SET] CASE Mixed|Lower|Upper [Respect|Ignore] [Respect|Ignore] [Respect|Ignore]

DESCRIPTION
     The CASE command sets the editor's handling of the case of text.

     The first option (which is mandatory) controls how text is entered
     by the user. When 'LOWER' or 'UPPER' are in effect, the shift or caps
     lock keys have no effect on the text being entered. When 'MIXED' is
     in effect, text is entered in the case set by the use of the shift
     and caps lock keys.

     The second option determines how the editor determines if a string
     target matches text in the file when the target is used in a <LOCATE>
     command.  With 'IGNORE' in effect, a match is
     found irrespective of the case of the target or the found text.
     The following strings are treated as equivalent: the THE The ThE...
     With 'RESPECT' in effect, the target and text must be the same case.
     Therefore a target of 'The' only matches text containing 'The', not
     'THE' or 'ThE' etc.

     The third option determines how the editor determines if a string
     target matches text in the file when the target is used in a <CHANGE>
     command.  With 'IGNORE' in effect, a match is
     found irrespective of the case of the target or the found text.
     The following strings are treated as equivalent: the THE The ThE...
     With 'RESPECT' in effect, the target and text must be the same case.
     Therefore a target of 'The' only matches text containing 'The', not
     'THE' or 'ThE' etc.

     The fourth option determines how the editor determines the sort
     order of upper and lower case with the <SORT> command.
     With 'IGNORE' in effect, upper and lower case letters are treated as
     equivalent.
     With 'RESPECT' in effect, upper and lower case letters are treated as
     different values and uppercase characters will sort before lowercase
     characters.

     All options can be specified as the current EQUIVCHAR to retain the
     existing value.

COMPATIBILITY
     XEDIT: Adds support for case significance in CHANGE commands.
     KEDIT: Adds support for LOWER option.
     Both:  Adds support for case significance in SORT command.

DEFAULT
     Mixed Ignore Respect Respect

SEE ALSO
     <SET EQUIVCHAR>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Case(CHARTYPE *params)
#else
short Case(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CAS_PARAMS  4
   CHARTYPE parm[CAS_PARAMS];
   CHARTYPE *word[CAS_PARAMS+1];
   CHARTYPE strip[CAS_PARAMS];
   register short i=0;
   short num_params=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Case");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   num_params = param_split(params,word,CAS_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   /*
    * Validate the first parameter: must be Mixed, Upper or Lower or EQUIVCHAR
    */
   parm[0] = (CHARTYPE)UNDEFINED_OPERAND;
   if (equal((CHARTYPE *)"mixed",word[0],1))
      parm[0] = CASE_MIXED;
   else if (equal((CHARTYPE *)"upper",word[0],1))
      parm[0] = CASE_UPPER;
   else if (equal((CHARTYPE *)"lower",word[0],1))
      parm[0] = CASE_LOWER;
   else if (equal((CHARTYPE *)EQUIVCHARstr,word[0],1))
      parm[0] = CURRENT_VIEW->case_enter;
   else
   {
      display_error(1,(CHARTYPE *)word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Save the current values of each remaining case setting.
    */
   parm[1] = CURRENT_VIEW->case_locate;
   parm[2] = CURRENT_VIEW->case_change;
   parm[3] = CURRENT_VIEW->case_sort;
   /*
    * Validate the remainder of the arguments.
    * Each must be Respect or Ignore, if present.
    */
   for (i=1;i<num_params;i++)
   {
      if (strcmp((DEFCHAR *)word[1],"") != 0)
      {
         if (equal((CHARTYPE *)"respect",word[i],1))
            parm[i] = CASE_RESPECT;
         else  if (equal((CHARTYPE *)"ignore",word[i],1))
            parm[i] = CASE_IGNORE;
         else  if (equal((CHARTYPE *)EQUIVCHARstr,word[i],1))
            parm[i] = parm[i];
         else
         {
            display_error(1,(CHARTYPE *)word[i],FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
         }
      }
   }
   /*
    * Set the new values of case settings for the view.
    */
   CURRENT_VIEW->case_enter  = parm[0];
   CURRENT_VIEW->case_locate = parm[1];
   CURRENT_VIEW->case_change = parm[2];
   CURRENT_VIEW->case_sort   = parm[3];

   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set clearerrorkey - specify which key clears the message line

SYNTAX
     [SET] CLEARErrorkey *|keyname

DESCRIPTION
     The SET CLEARERRORKEY command allows the user to specify which
     key clears the message line.  By default, any key pressed will
     cause the message line to be cleared.  The keyname specified
     is the name returned via the <SHOWKEY> command.

     As the <QUERY> command also uses the same mechanism for displaying
     its results as errors, then this command affects when results from
     the <QUERY> command are cleared.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     *

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Clearerrorkey(CHARTYPE *params)
#else
short Clearerrorkey(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
   int key = 0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Clearerrorkey");
   if (strcmp((DEFCHAR*)params,"*") == 0)
   {
      CLEARERRORKEYx = -1;
   }
   else
   {
      key = find_key_value(params);
      if (key == -1)
      {
         display_error(13,params,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      CLEARERRORKEYx = key;
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set clearscreen - indicate if the screen is to be cleared on exit

SYNTAX
     [SET] CLEARScreen ON|OFF

DESCRIPTION
     The SET CLEARSCREEN command allows the user to request that the
     screen be cleared on exit from THE.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     OFF

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Clearscreen(CHARTYPE *params)
#else
short Clearscreen(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Clearscreen");
 rc = execute_set_on_off(params,&CLEARSCREENx);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set clock - turn on or off display of time on status line

SYNTAX
     [SET] CLOCK ON|OFF

DESCRIPTION
     The SET CLOCK command turns on or off the display of the time on the
     <status line>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     ON

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Clock(CHARTYPE *params)
#else
short Clock(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Clock");
 rc = execute_set_on_off(params,&CLOCKx);
 if (rc == RC_OK
 &&  curses_started)
    clear_statarea();
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set cmdarrows - sets the behaviour of the up and down arrow keys

SYNTAX
     [SET] CMDArrows Retrieve|Tab

DESCRIPTION
     The SET CMDARROWS command determines the action that occurs when the
     up and down arrows keys are hit while on the <command line>.

     'RETRIEVE' will set the up and down arrows to retrieve the last or
     next command entered on the <command line>.

     'TAB' will set the up and down arrows to move to the last
     or first line respectively of the main window.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     RETRIEVE

SEE ALSO
     <CURSOR>, <?>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Cmdarrows(CHARTYPE *params)
#else
short Cmdarrows(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Cmdarrows");
/*---------------------------------------------------------------------*/
/* Determine values for first parameter; command line behaviour        */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"tab",params,1))
    CMDARROWSTABCMDx = TRUE;
 else
   if (equal((CHARTYPE *)"retrieve",params,1))
      CMDARROWSTABCMDx = FALSE;
   else
     {
      display_error(1,params,FALSE);
      rc = RC_INVALID_OPERAND;
     }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set cmdline - sets the position of the command line.

SYNTAX
     [SET] CMDline ON|OFF|Top|Bottom

DESCRIPTION
     The SET CMDLINE command sets the position of the <command line>,
     either at the top of the screen, the bottom of the screen or off.

COMPATIBILITY
     XEDIT: Compatible.
            CMDLINE ON is equivalent to CMDLINE Bottom
     KEDIT: Compatible.

DEFAULT
     BOTTOM

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Cmdline(CHARTYPE *params)
#else
short Cmdline(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   CHARTYPE cmd_place = '?';
   short rc = RC_OK;

   TRACE_FUNCTION( "commset1.c:Cmdline" );
   if ( equal( (CHARTYPE *)"top", params, 1 ) )
   {
      cmd_place = 'T';
   }
   if ( equal( (CHARTYPE *)"bottom",params, 1 )
   ||   equal( (CHARTYPE *)"on", params, 2 ) )
   {
      cmd_place = 'B';
   }
   if ( equal( (CHARTYPE *)"off", params, 3 ) )
   {
      cmd_place = 'O';
   }
   if ( cmd_place == '?' )
   {
      display_error( 1, (CHARTYPE *)params, FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If the setting supplied is the same as the current setting, just
    * return without doing anything.
    */
   if ( cmd_place == CURRENT_VIEW->cmd_line )
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * Now we need to move the windows around.
    */
   CURRENT_VIEW->cmd_line = cmd_place;
   /*
    * Rebuild the windows and display...
    */
   set_screen_defaults();
   if ( curses_started )
   {
      if ( set_up_windows( current_screen ) != RC_OK )
      {
         TRACE_RETURN();
         return(rc);
      }
   }
   if (CURRENT_VIEW->cmd_line == 'O')
      CURRENT_VIEW->current_window = WINDOW_FILEAREA;
   build_screen( current_screen );
   if ( curses_started )
      display_screen( current_screen );

   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set color - set colours for display

SYNTAX
     [SET] COLOR  area [modifier[...]] [foreground] [ON] [background]
     [SET] COLOR  area [modifier[...]] ON|OFF

DESCRIPTION
     The SET COLOR command changes the colours or display attributes of
     various display areas in THE.

     Valid values for 'area':

          ALERT      - alert boxes; see <ALERT>
          Arrow      - command line prompt
          Block      - marked <block>
          BOUNDmarker- bound markers (GUI platforms only)
          CBlock     - <current line> if in marked <block>
          CHIghlight - highlighted line if the same as <current line>
          Cmdline    - <command line>
          CTofeof    - as for TOfeof if the same as <current line>
          CUrline    - the <current line>
          DIALOG     - dialog boxes; see <DIALOG>
          Divider    - dividing line between vertical split screens
          Filearea   - area containing file lines
          GAP        - the gap between the <prefix area> and <filearea>
          CGAP       - the gap between the <prefix area> and <filearea> - current
          HIghlight  - highlighted line
          Idline     - line containing file specific info
          Msgline    - error messages
          Nondisp    - Non-display characters (<SET ETMODE> OFF)
          Pending    - pending commands in <prefix area>
          PRefix     - <prefix area>
          CPRefix    - <prefix area> if the same as <current line>
          Reserved   - default for <reserved line>
          Scale      - line showing <scale line>
          SHadow     - hidden line marker lines
          SLK        - soft label keys
          STatarea   - line showing status of editing session
          Tabline    - line showing tab positions
          TOfeof     - <Top-of-File line> and <Bottom-of-File line>
          *          - All area (second format only)

     Valid values for 'foreground' and 'background':

          BLAck
          BLUe
          Brown
          Green
          GRAy
          GREy
          Cyan
          RED
          Magenta
          Pink
          Turquoise
          Yellow
          White

     Valid values for 'modifier':

          NORmal
          BLInk
          BOld
          BRIght
          High
          REVerse
          Underline
          DARK
          Italic - only available on X11 port with valid Italic font

     The second format of this command allows the user to turn on or off
     any of the valid modifiers.

     It is an error to attempt to set a colour on a mono display.

COMPATIBILITY
     XEDIT: Functionally compatible. See below.
     KEDIT: Functionally compatible. See below.
     Does not implement all modifiers.

DEFAULT
     Depends on compatibility mode setting and monitor type.

SEE ALSO
     <SET COMPAT>, <SET COLOUR>, <SET ECOLOUR>

STATUS
     Complete.
**man-end**********************************************************************/
/*man-start*********************************************************************
COMMAND
     set colour - set colours for display

SYNTAX
     [SET] COLOUR area [modifier[...]] [foreground] [on background]
     [SET] COLOUR area [modifier[...]] ON|OFF

DESCRIPTION
     The SET COLOUR command is a synonym for the <SET COLOR> command.

COMPATIBILITY
     XEDIT: Functionally compatible. See below.
     KEDIT: Functionally compatible. See below.
     Does not implement all modifiers.

DEFAULT
     Depends on compatibility mode setting and monitor type.

SEE ALSO
     <SET COLOR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Colour(CHARTYPE *params)
#else
short Colour(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define COL_PARAMS 2
#define COL_MODIFIER_NO_SET  0
#define COL_MODIFIER_SET_ON  1
#define COL_MODIFIER_SET_OFF 2
   CHARTYPE *word[COL_PARAMS+1];
   CHARTYPE strip[COL_PARAMS];
   CHARTYPE parm[COL_PARAMS];
   register short i=0;
   unsigned short num_params=0;
   short area=-1;
   COLOUR_ATTR attr,tmp_attr;
   CHARTYPE *dummy=NULL;
   bool any_colours=FALSE;
   short word1_len,modifier_set=COL_MODIFIER_NO_SET;
   bool window_set[MAX_THE_WINDOWS];
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Colour");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   num_params = param_split(params,word,COL_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 2 )
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Check which format of this command we are running.
    * If the last word is ON or OFF then we are executing the
    * second format.
    */
   word1_len = strlen( (DEFCHAR *)word[1] );
   if ( my_stricmp( (DEFCHAR *)word[1]+word1_len-2, "ON" ) == 0 )
      modifier_set = COL_MODIFIER_SET_ON;
   else if ( my_stricmp( (DEFCHAR *)word[1]+word1_len-3, "OFF" ) == 0 )
      modifier_set = COL_MODIFIER_SET_OFF;
   if ( modifier_set )
   {
      /*
       * Check that first parameter is an area or '*'
       */
      parm[0] = FALSE;
      if ( strcmp( (DEFCHAR *)word[0], "*" ) == 0 )
      {
         area = -1;
         parm[0] = TRUE;
      }
      else
      {
         for ( i = 0; i < ATTR_MAX; i++ )
         {
            if (equal(valid_areas[i].area,word[0],valid_areas[i].area_min_len))
            {
               parm[0] = TRUE;
               area = i;
               break;
            }
         }
      }
      if (parm[0] == FALSE)
      {
         display_error(1,(CHARTYPE *)word[0],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      /*
       * Check that each subsequent parameter (except the last) is
       * a modifier.
       */
      if ( parse_modifiers( word[1], &tmp_attr ) != RC_OK )
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      /*
       * For each area, turn off the modifiers and redraw the affected part of
       * the screen
       */
      if ( area == (-1) )
      {
         for ( i = 0; i < MAX_THE_WINDOWS; i++ )
         {
            window_set[i] = FALSE;
         }
         for ( i = 0; i < ATTR_MAX; i++ )
         {
            attr = CURRENT_FILE->attr[i];
            if ( modifier_set == COL_MODIFIER_SET_ON )
            {
               if ( colour_support )
                  attr.mod |= tmp_attr.mod;
               else
                  attr.mono |= tmp_attr.mono;
            }
            else
            {
               if ( colour_support )
                  attr.mod &= ~tmp_attr.mod;
               else
                  attr.mono &= ~tmp_attr.mono;
            }
            CURRENT_FILE->attr[i] = attr;
            if ( i == ATTR_BOUNDMARK
            ||   i == ATTR_NONDISP
            ||   window_set[valid_areas[i].area_window] == FALSE )
            {
               set_active_colour( i );
               window_set[valid_areas[i].area_window] = TRUE;
            }
         }
      }
      else
      {
         attr = CURRENT_FILE->attr[area];
         if ( modifier_set == COL_MODIFIER_SET_ON )
         {
            if ( colour_support )
               attr.mod |= tmp_attr.mod;
            else
               attr.mono |= tmp_attr.mono;
         }
         else
         {
            if ( colour_support )
               attr.mod &= ~tmp_attr.mod;
            else
               attr.mono &= ~tmp_attr.mono;
         }
         CURRENT_FILE->attr[area] =attr;
         set_active_colour( area );
      }
   }
   else
   {
      /*
       * Check that the supplied area matches one of the values in the area
       * array and that the length is at least as long as the minimum.
       */
      parm[0] = FALSE;
      for (i=0;i<ATTR_MAX;i++)
      {
         if (equal(valid_areas[i].area,word[0],valid_areas[i].area_min_len))
         {
            parm[0] = TRUE;
            area = i;
            break;
         }
      }
      if (parm[0] == FALSE)
      {
         display_error(1,(CHARTYPE *)word[0],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      attr = CURRENT_FILE->attr[area];
      /*
       * Determine colours and modifiers.
       */
      if (parse_colours(word[1],&attr,&dummy,FALSE,&any_colours) != RC_OK)
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      /*
       * Now we have the new colours, save them with the current file...
       */
      CURRENT_FILE->attr[area] = attr;
      set_active_colour( area );
   }
   TRACE_RETURN();
   return(RC_OK);
}

/*man-start*********************************************************************
COMMAND
     set coloring - enable or disable syntax highlighting

SYNTAX
     [SET] COLORING ON|OFF [AUTO|parser]

DESCRIPTION
     The SET COLORING command allows the user to turn on or off syntax
     highlighting for current file.  It also allows the <parser> used to be
     specified explicitly, or automatically determined by the file
     extension or <magic number>.

     ON turns on syntax highlighting for the current file, OFF turns it
     off.

     AUTO determines the <parser> to use for the current file based on the
     file extension.  The <parser> to use is controlled by the <SET AUTOCOLOR>
     command.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     ON AUTO

SEE ALSO
     <SET COLOURING>, <SET ECOLOUR>, <SET AUTOCOLOR>, <SET PARSER>

STATUS
     Complete.
**man-end**********************************************************************/
/*man-start*********************************************************************
COMMAND
     set colouring - enable or disable syntax highlighting

SYNTAX
     [SET] COLOURING ON|OFF [AUTO|parser]

DESCRIPTION
     The SET COLOURING command is a synonym for the <SET COLORING> command.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     ON AUTO

SEE ALSO
     <SET COLORING>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Colouring(CHARTYPE *params)
#else
short Colouring(params)
CHARTYPE *params;
#endif

/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define COLG_PARAMS  2
 CHARTYPE *word[COLG_PARAMS+1];
 CHARTYPE strip[COLG_PARAMS];
 short num_params=0;
 short rc=RC_OK;
 bool new_colouring=FALSE;
 PARSER_DETAILS *new_parser=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Colouring");
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,COLG_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
 {
    display_error(3,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
 }
/*---------------------------------------------------------------------*/
/* Parse the status parameter...                                       */
/*---------------------------------------------------------------------*/
 rc = execute_set_on_off(word[0],&new_colouring);
 if (rc != RC_OK)
 {
    TRACE_RETURN();
    return(rc);
 }
 if (num_params == 1
 &&  new_colouring == TRUE)
 {
    display_error(3,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
 }

 if (new_colouring)
 {
    /*
     * This is only applicable when turning colouring ON
     */
    if (equal((CHARTYPE *)"AUTO",word[1],4))
    {
       /*
        * Set the parser to the parser for the file extension or
        * to NULL if no parser is set up for this extension.
        */
       new_parser = find_auto_parser(CURRENT_FILE);
       CURRENT_FILE->autocolour = TRUE;
    }
    else
    {
       /*
        * Look for a parser with the specified name
        */
       new_parser = parserll_find(first_parser,word[1]);
       if (new_parser == NULL) /* no parser by that name... */
       {
          display_error(199,word[1],FALSE);
          TRACE_RETURN();
          return RC_INVALID_OPERAND;
       }
       CURRENT_FILE->autocolour = FALSE;
    }
 }
 CURRENT_FILE->parser = new_parser;
 CURRENT_FILE->colouring = new_colouring;
 /*
  * If all is OK, redisplay the screen to get the new colouring
  */
 if (display_screens > 1
 &&  SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen))
 {
    display_screen(other_screen);
 }
 display_screen(current_screen);
 TRACE_RETURN();
 return(RC_OK);
}

/*man-start*********************************************************************
COMMAND
     set compat - set compatibility mode

SYNTAX
     [SET] COMPat The|Xedit|Kedit|KEDITW|Ispf|= [The|Xedit|Kedit|KEDITW|Ispf|=] [The|Xedit|Kedit|KEDITW|Ispf|=]

DESCRIPTION
     The SET COMPAT command changes some settings of THE to make it
     more compatible with the look and/or feel of XEDIT, KEDIT,
     KEDIT for Windows, or ISPF.

     This command is most useful as the first <SET> command in a
     profile file. It will change the default settings of THE to
     initially look like the chosen editor. You can then make any
     additional changes in THE by issuing other <SET> commands.

     It is recommended that this command NOT be executed from the
     command line, particularly if you have 2 files being displayed
     at the same time.  Although the command works, things may look
     and behave strangely :-)

     The first parameter affects the look of THE, the second parameter
     affects the feel of THE, and the third parameter determines
     which default function key settings you require.

     Any of the parameters can be specified as =, which will not
     change that aspect of THE's compatability.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     THE THE THE

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Compat(CHARTYPE *params)
#else
short Compat(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define COM_PARAMS  4
   CHARTYPE *word[COM_PARAMS+1];
   CHARTYPE strip[COM_PARAMS];
   short num_params=0;
   int rc=RC_OK;
   int prey=0,prex=0;
   short save_look=compatible_look;
   short save_feel=compatible_feel;
   short save_keys=compatible_keys;
   short new_look=0;
   short new_feel=0;
   short new_keys=0;
   unsigned short save_autosave_alt=0;
   unsigned short save_save_alt=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Compat");
   /*
    * Parse the parameters...
    */
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_NONE;
   num_params = param_split(params,word,COM_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 1)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (num_params > 3)
   {
      display_error(2,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (equal((CHARTYPE *)"the",word[0],1))
      new_look = COMPAT_THE;
   else  if (equal((CHARTYPE *)"xedit",word[0],1))
      new_look = COMPAT_XEDIT;
   else if (equal((CHARTYPE *)"kedit",word[0],1))
      new_look = COMPAT_KEDIT;
   else if (equal((CHARTYPE *)"keditw",word[0],6))
      new_look = COMPAT_KEDITW;
   else if (equal((CHARTYPE *)"ispf",word[0],1))
      new_look = COMPAT_ISPF;
   else if (equal((CHARTYPE *)EQUIVCHARstr,word[0],1))
      new_look = save_look;
   else
   {
      display_error(1,word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (num_params == 1)
   {
      new_feel = save_feel;
      new_keys = save_keys;
   }
   else
   {
      if (equal((CHARTYPE *)"the",word[1],1))
         new_feel = COMPAT_THE;
      else if (equal((CHARTYPE *)"xedit",word[1],1))
         new_feel = COMPAT_XEDIT;
      else if (equal((CHARTYPE *)"kedit",word[1],1))
         new_feel = COMPAT_KEDIT;
      else if (equal((CHARTYPE *)"keditw",word[1],6))
         new_feel = COMPAT_KEDITW;
      else if (equal((CHARTYPE *)"ispf",word[1],1))
         new_feel = COMPAT_ISPF;
      else if (equal((CHARTYPE *)EQUIVCHARstr,word[1],1))
         new_feel = save_feel;
      else
      {
         display_error(1,word[1],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      if (num_params == 2)
         new_keys = save_keys;
      else
      {
         if (equal((CHARTYPE *)"the",word[2],1))
            new_keys = COMPAT_THE;
         else if (equal((CHARTYPE *)"xedit",word[2],1))
            new_keys = COMPAT_XEDIT;
         else if (equal((CHARTYPE *)"kedit",word[2],1))
            new_keys = COMPAT_KEDIT;
         else if (equal((CHARTYPE *)"keditw",word[2],6))
            new_keys = COMPAT_KEDITW;
         else if (equal((CHARTYPE *)"ispf",word[2],1))
            new_keys = COMPAT_ISPF;
         else if (equal((CHARTYPE *)EQUIVCHARstr,word[2],1))
            new_keys = save_keys;
         else
         {
            display_error(1,word[2],FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
         }
      }
   }
   compatible_look = new_look;
   compatible_feel = new_feel;
   compatible_keys = new_keys;
   /*
    * If the FEEL has changed, change the default feel...
    */
   set_global_feel_defaults();
   /*
    * If the KEYS has changed, change the default key definitions...
    */
   if (save_keys != compatible_keys)
   {
      switch(compatible_keys)
      {
         case COMPAT_THE:
            rc = set_THE_key_defaults(prey,prex);
            break;
         case COMPAT_XEDIT:
            rc = set_XEDIT_key_defaults(prey,prex);
            break;
         case COMPAT_ISPF:
            rc = set_ISPF_key_defaults(prey,prex);
            break;
         case COMPAT_KEDIT:
         case COMPAT_KEDITW:
            rc = set_KEDIT_key_defaults(prey,prex);
            break;
      }
      if (rc != RC_OK)
      {
         TRACE_RETURN();
         return(rc);
      }
   }
   /*
    * Now we have to change the LOOK of the current view...
    */
   if (curses_started)
   {
      if (CURRENT_WINDOW_PREFIX != NULL)
         getyx(CURRENT_WINDOW_PREFIX,prey,prex);
   }
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   /*
    * Reset common settings to defaults for THE...
    */
   set_global_look_defaults();

#ifdef FOR_ALL_VIEWS
   /*
    * Change the settings for all views.
    */
   viewp = vd_first;
   while(viewp != NULL)
   {
      set_file_defaults(viewp->file_for_view);
      set_view_defaults(viewp);
      viewp = viewp->next;
   }
#else
   save_autosave_alt = CURRENT_FILE->autosave_alt;
   save_save_alt = CURRENT_FILE->save_alt;
   set_file_defaults(CURRENT_FILE);
   CURRENT_FILE->autosave_alt = save_autosave_alt;
   CURRENT_FILE->save_alt = save_save_alt;
   set_view_defaults(CURRENT_VIEW);
#endif
   /*
    * Determine the size of each window in each screen in case any changes
    * in defaults caused some settings to include/exclude some windows...
    */
   set_screen_defaults();
   /*
    * For the common windows, set their attributes to match the new values
    */
   if (curses_started
   &&  statarea != NULL)
   {
      wattrset(statarea,set_colour(CURRENT_FILE->attr+ATTR_STATAREA));
      clear_statarea();
   }
   /*
    * If more than one screen displayed, redisplay the 'other' screen...
    */
   if (display_screens > 1)
   {
      OTHER_SCREEN.screen_view->current_row = calculate_actual_row(OTHER_SCREEN.screen_view->current_base,
                                                  OTHER_SCREEN.screen_view->current_off,
                                                  OTHER_SCREEN.rows[WINDOW_FILEAREA],TRUE);
      pre_process_line(OTHER_SCREEN.screen_view,OTHER_SCREEN.screen_view->focus_line,(_LINE *)NULL);
      if (OTHER_SCREEN.screen_view->cmd_line == 'O')
         OTHER_SCREEN.screen_view->current_window = WINDOW_FILEAREA;
      if (curses_started)
      {
         if (set_up_windows(current_screen) != RC_OK)
         {
            TRACE_RETURN();
            return(rc);
         }
         if (!horizontal)
         {
            wattrset(divider,set_colour(OTHER_SCREEN.screen_view->file_for_view->attr+ATTR_DIVIDER));
            touchwin(divider);
            wnoutrefresh(divider);
         }
      }
      redraw_screen((current_screen == 0)?1:0);
      build_screen(other_screen);
      display_screen(other_screen);
   }
   /*
    * Redisplay the current screen...
    */
   CURRENT_VIEW->current_row = calculate_actual_row(CURRENT_VIEW->current_base,
                                                  CURRENT_VIEW->current_off,
                                                  CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE);
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
   if (CURRENT_VIEW->cmd_line == 'O')
      CURRENT_VIEW->current_window = WINDOW_FILEAREA;
   if (curses_started)
   {
      if (set_up_windows(current_screen) != RC_OK)
      {
         TRACE_RETURN();
         return(rc);
      }
   }
   redraw_screen(current_screen);
   build_screen(current_screen);
   display_screen(current_screen);

   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set ctlchar - define control character attributes

SYNTAX
     [SET] CTLchar OFF
     [SET] CTLchar char Escape | OFF
     [SET] CTLchar char Protect|Noprotect [modifier[...]] [fore [ON back]]

DESCRIPTION
     The SET CTLCHAR command defines control characters to be used when
     displaying a <reserved line>.  Control characters determine how parts
     of a <reserved line> are displayed.

     See <SET COLOUR> for valid values for 'modifier', 'fore' and 'back'.

     The 'Protect' and 'Noprotect' arguments are ignored.

COMPATIBILITY
     XEDIT: Similar, but does not support all parameters.
     KEDIT: N/A.

DEFAULT
     OFF

SEE ALSO
     <SET COLOUR>, <SET RESERVED>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Ctlchar(CHARTYPE *params)
#else
short Ctlchar(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CTL_PARAMS  3
 CHARTYPE *word[CTL_PARAMS+1];
 CHARTYPE strip[CTL_PARAMS];
 short num_params=0;
 COLOUR_ATTR attr;
 CHARTYPE *dummy=NULL;
 bool any_colours=FALSE,protect,found;
 int i;
 bool have_ctlchar=TRUE;
 RESERVED *curr;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Ctlchar");
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_BOTH;
 num_params = param_split(params,word,CTL_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
 {
    display_error(3,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
 }
 if (num_params > 3)
 {
    display_error(2,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
 }
 if (num_params == 1)
 {
    if (equal((CHARTYPE *)"off",word[0],1))
    {
       if (num_params != 1)
       {
          display_error(2,(CHARTYPE *)"",FALSE);
          TRACE_RETURN();
          return(RC_INVALID_OPERAND);
       }
       have_ctlchar = FALSE;
    }
    else
    {
       display_error(1,word[0],FALSE);
       TRACE_RETURN();
       return(RC_INVALID_OPERAND);
    }
 }
 else
 {
    if (strlen((DEFCHAR *)word[0]) > 1)
    {
       display_error(37,word[0],FALSE);
       TRACE_RETURN();
       return(RC_INVALID_OPERAND);
    }
    if (num_params == 2)
    {
       if (equal((CHARTYPE *)"escape",word[1],1))
       {
          /*
           * Sets the value in word[0] to be the escape character
           */
          ctlchar_escape = word[0][0];
       }
       else if (equal((CHARTYPE *)"off",word[1],3))
       {
          /*
           * Turns off the escape character in word[0]
           * Find the entry in
           */
          for (i=0;i<MAX_CTLCHARS;i++)
          {
             if (ctlchar_char[i] == word[0][0])
             {
                ctlchar_char[i] = 0;
                break;
             }
          }
       }
       else
       {
          display_error(1,word[1],FALSE);
          TRACE_RETURN();
          return(RC_INVALID_OPERAND);
       }
    }
    else
    {
       /*
        * Now should be parsing colours to set the ctlchar colour for
        * the character in word[0][0]
        */
       if (equal((CHARTYPE *)"protect",word[1],1))
          protect = TRUE;
       else if (equal((CHARTYPE *)"noprotect",word[1],1))
          protect = FALSE;
       else
       {
          display_error(1,word[1],FALSE);
          TRACE_RETURN();
          return(RC_INVALID_OPERAND);
       }
       memset(&attr,0,sizeof(COLOUR_ATTR));
       if (parse_colours(word[2],&attr,&dummy,FALSE,&any_colours) != RC_OK)
       {
          TRACE_RETURN();
          return(RC_INVALID_OPERAND);
       }
       /*
        * Find any existing CTLCHAR spec for the supplied character
        * and turn it off...
        */
       for (i=0;i<MAX_CTLCHARS;i++)
       {
          if (ctlchar_char[i] == word[0][0])
          {
             ctlchar_char[i] = 0;
             break;
          }
       }
       /*
        * Find the first spare CTLCHAR spec for the supplied character
        * and add it.
        */
       found = FALSE;
       for (i=0;i<MAX_CTLCHARS;i++)
       {
          if (ctlchar_char[i] == 0)
          {
             ctlchar_char[i] = word[0][0];
             ctlchar_attr[i] = attr;
             found = TRUE;
             break;
          }
       }
       if (!found)
       {
          display_error(80,(CHARTYPE *)"",FALSE);
          TRACE_RETURN();
          return(RC_INVALID_OPERAND);
       }
    }
 }
 CTLCHARx = have_ctlchar;
 /*
  * For each current reserved line, reparse it to ensure the changes made
  * here are reflected correctly.
  */
 curr = CURRENT_FILE->first_reserved;
 while(curr)
 {
    parse_reserved_line(curr);
    curr = curr->next;
 }
 if (display_screens > 1
 &&  SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen))
 {
    build_screen(other_screen);
    display_screen(other_screen);
 }
 build_screen(current_screen);
 display_screen(current_screen);
 TRACE_RETURN();
 return(RC_OK);
}

/*man-start*********************************************************************
COMMAND
     set curline - set position of current line on screen

SYNTAX
     [SET] CURLine M[+n|-n] | [+|-]n

DESCRIPTION
     The SET CURLINE command sets the position of the <current line> to
     the physical screen line specified by supplied arguments.

     The first form of parameters is:

     M[+n|-n]
     this sets the <current line> to be relative to the middle of
     the screen. A positive value adds to the middle line number,
     a negative subtracts from it.
     eg. M+3 on a 24 line screen will be line 15
         M-5 on a 24 line screen will be line 7

     The second form of parameters is:

     [+|-]n
     this sets the <current line> to be relative to the top of the
     screen (if positive or no sign) or relative to the bottom
     of the screen if negative.
     eg. +3 or 3 will set current line to line 3
         -3 on a 24 line screen will be line 21

     If the resulting line is outside the bounds of the screen
     the position of the current line will become the middle line
     on the screen.

     It is an error to try to position the CURLINE on the same
     line as a line already allocated by one of <SET HEXSHOW>,
     <SET RESERVED>, <SET SCALE> or <SET TABLINE>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     M

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Curline(CHARTYPE *params)
#else
short Curline(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CUR_PARAMS  1
 CHARTYPE *word[CUR_PARAMS+1];
 CHARTYPE strip[CUR_PARAMS];
 short num_params=0;
 short rc=0;
 short base = (short)CURRENT_VIEW->current_base;
 short off = CURRENT_VIEW->current_off;
 short hexshow_row=0,curline_row=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Curline");
 strip[0]=STRIP_BOTH;
 num_params = param_split(params,word,CUR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 if (num_params > 1)
   {
    display_error(2,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Parse the parameter...                                              */
/*---------------------------------------------------------------------*/
 rc = execute_set_row_position(params,&base,&off);
 if (rc != RC_OK)
   {
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* If the CURLINE is the same line as HEXSHOW, SCALE, TABLE or has a   */
/* RESERVED line on it, return ERROR.                                  */
/*---------------------------------------------------------------------*/
 curline_row = calculate_actual_row(base,off,CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE);
 if (calculate_actual_row(CURRENT_VIEW->scale_base,
                          CURRENT_VIEW->scale_off,
                          CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE) == curline_row
 && CURRENT_VIEW->scale_on)
   {
    display_error(64,(CHARTYPE *)"- same as SCALE",FALSE);
    TRACE_RETURN();
    return(rc);
   }
 if (calculate_actual_row(CURRENT_VIEW->tab_base,
                          CURRENT_VIEW->tab_off,
                          CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE) == curline_row
 && CURRENT_VIEW->tab_on)
   {
    display_error(64,(CHARTYPE *)"- same as TABLINE",FALSE);
    TRACE_RETURN();
    return(rc);
   }
 hexshow_row = calculate_actual_row(CURRENT_VIEW->hexshow_base,
                                    CURRENT_VIEW->hexshow_off,
                                    CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE);
 if ((hexshow_row == curline_row
    ||  hexshow_row + 1 == curline_row)
 && CURRENT_VIEW->hexshow_on)
   {
    display_error(64,(CHARTYPE *)"- same as HEXSHOW",FALSE);
    TRACE_RETURN();
    return(rc);
   }
 if (find_reserved_line(current_screen,TRUE,curline_row,0,0) != NULL)
   {
    display_error(64,(CHARTYPE *)"- same as RESERVED line",FALSE);
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* If the "real" row for CURLINE is not the same as the generated one, */
/* set the base and offset to reflect the generated row.               */
/*---------------------------------------------------------------------*/
 if (calculate_actual_row(base,off,CURRENT_SCREEN.rows[WINDOW_FILEAREA],FALSE) != curline_row)
   {
    CURRENT_VIEW->current_base = (CHARTYPE)POSITION_MIDDLE;
    CURRENT_VIEW->current_off = 0;
   }
 else
   {
    CURRENT_VIEW->current_base = (CHARTYPE)base;
    CURRENT_VIEW->current_off = off;
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
 CURRENT_VIEW->current_row = curline_row;
 build_screen(current_screen);
 display_screen(current_screen);

 TRACE_RETURN();
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set cursorstay - set on or off the behaviour of the cursor on a scroll

SYNTAX
     [SET] CURSORSTay ON|OFF

DESCRIPTION
     The SETCURSORSTAY command allows the user to set the behaviour of
     the cursor when the file is scrolled with a <FORWARD> or <BACKWARD>
     command.

     Before this command was introduced, the position of the cursor
     after the file was scrolled depended on <SET COMPAT>; for
     THE, the cursor moved to the current line, for XEDIT and KEDIT
     modes the cursor stayed on the same screen line.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     ON

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short CursorStay(CHARTYPE *params)
#else
short CursorStay(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:CursorStay");
 rc = execute_set_on_off(params,&scroll_cursor_stay);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set defsort - specify the order in which files appear in DIR.DIR

SYNTAX
     [SET] DEFSORT OFF|DIRectory|Size|Date|Time|Name [Ascending|Descending]

DESCRIPTION
     The SET DEFSORT command allows the user to determine the order
     in which files appear in a DIR.DIR file.

     'Directory' specifies that directories within the current directory
     are shown before other files.

     'Size' specifies that the size of the file determines the order
     in which files are displayed.

     'Date' specifies that the date of the last change to the file
     determines the order in which files are displayed. If the dates
     are the same, the time the file was last changed is used as a
     secondary sort key.

     'Time' specifies that the time of the file determines the order
     in which files are displayed.

     'Name' specifies that the name of the file determines the order in
     which files are displayed. This is the default.  Files are sorted
     by name as a secondary sort key when any of the above options are
     specified and two files have equal values for that sort option.

     'OFF' indicates that no ordering of the files in the directory
     is performed.  On directories with a large number of files, this
     option results in a displayed DIR.DIR file much quicker than any
     sorted display.

     The second parameter specifies if the sort order is ascending or
     descending.

     This command does not affect how any current DIR.DIR file is shown
     but is applicable the next time a directory is displayed as a
     result of a DIR or LS command.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Similar in functionality.

DEFAULT
     NAME ASCENDING

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Defsort(CHARTYPE *params)
#else
short Defsort(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define DIR_PARAMS  2
 CHARTYPE *word[DIR_PARAMS+1];
 CHARTYPE strip[DIR_PARAMS];
 short num_params=0;
 short rc=RC_OK;
 int defsort=0;
 int dirorder=DIRSORT_ASC;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Defsort");
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,DIR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
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
 if (equal((CHARTYPE *)"directory",word[0],3))
    defsort = DIRSORT_DIR;
 else
    if (equal((CHARTYPE *)"name",word[0],1))
       defsort = DIRSORT_NAME;
    else
       if (equal((CHARTYPE *)"time",word[0],1))
          defsort = DIRSORT_TIME;
       else
          if (equal((CHARTYPE *)"size",word[0],1))
             defsort = DIRSORT_SIZE;
          else
             if (equal((CHARTYPE *)"date",word[0],1))
                defsort = DIRSORT_DATE;
             else
                if (equal((CHARTYPE *)"off",word[0],3))
                   defsort = DIRSORT_NONE;
                else
                  {
                   display_error(1,(CHARTYPE *)word[0],FALSE);
                   TRACE_RETURN();
                   return(RC_INVALID_OPERAND);
                  }
 if (num_params == 2)
   {
    if (equal((CHARTYPE *)"ascending",word[1],1))
       dirorder = DIRSORT_ASC;
    else
       if (equal((CHARTYPE *)"descending",word[1],1))
          dirorder = DIRSORT_DESC;
       else
         {
          display_error(1,(CHARTYPE *)word[1],FALSE);
          TRACE_RETURN();
          return(RC_INVALID_OPERAND);
         }
   }
 DEFSORTx = defsort;
 DIRORDERx = dirorder;
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set dirinclude - set the file mask for directory command

SYNTAX
     [SET] DIRInclude *
     [SET] DIRInclude [Normal] [Readonly] [System] [Hidden] [Directory]

DESCRIPTION
     The DIRINCLUDE command sets the file mask for files that will be
     displayed on subsequent DIRECTORY commands. The operand "*" will
     set the mask to all files, the other options will set the
     mask to include those options specified together with "normal"
     files eg.

        DIRINCLUDE R S

     will display readonly and system files together with "normal" files
     the next time the DIRECTORY command is issued.

     The effects of DIRINCLUDE are ignored in the Unix version.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     *

SEE ALSO
     <DIRECTORY>, <LS>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Dirinclude(CHARTYPE *params)
#else
short Dirinclude(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Dirinclude");
 rc = set_dirtype(params);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set display - specify which level of lines to display

SYNTAX
     [SET] DISPlay n [m|*]

DESCRIPTION
     The SET DISPLAY command sets the selection level for lines to be
     displayed on the screen.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     0 0

SEE ALSO
     <SET SCOPE>, <SET SELECT>, <ALL>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Display(CHARTYPE *params)
#else
short Display(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 short col1=0,col2=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Display");
 if ((rc = validate_n_m(params,&col1,&col2)) != RC_OK)
   {
    TRACE_RETURN();
    return(rc);
   }
 CURRENT_VIEW->display_low = col1;
 CURRENT_VIEW->display_high = col2;
/*---------------------------------------------------------------------*/
/* If we are on the command line and the result of this statement means*/
/* that the current line is no longer in scope, we need to make the    */
/* current line and possibly the focus line the next line in scope.    */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
   {
    CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW,NULL,get_true_line(TRUE),DIRECTION_FORWARD);
    build_screen(current_screen);
    if (!line_in_view(current_screen,CURRENT_VIEW->focus_line))
      {
       CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
       pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
      }
   }
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
 build_screen(current_screen);
 display_screen(current_screen);

 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set ecolor - set colors for syntax highlighting

SYNTAX
     [SET] ECOLOR char [modifier[...]] [foreground] [on background]
     [SET] ECOLOR char [modifier[...]] ON|OFF

DESCRIPTION
     The SET ECOLOR command allows the user to specify the colors of
     each category of items used in syntax highlighting.

     'char' refers to one of the following valid values:

     A - comments
     B - strings
     C - numbers
     D - keywords
     E - labels
     F - preprocessor directives
     G - header lines
     H - extra right paren, matchable keyword (N/A)
     I - level 1 paren
     J - level 1 matchable keyword (N/A)
     K - level 1 matchable preprocessor keyword (N/A)
     L - level 2 paren, matchable keyword (N/A)
     M - level 3 paren, matchable keyword (N/A)
     N - level 4 paren, matchable keyword (N/A)
     O - level 5 paren, matchable keyword (N/A)
     P - level 6 paren, matchable keyword (N/A)
     Q - level 7 paren, matchable keyword (N/A)
     R - level 8 paren or higher, matchable keyword (N/A)
     S - incomplete string
     T - HTML markup tags
     U - HTML character/entity references
     V - Builtin functions
     W - not used
     X - not used
     Y - not used
     Z - not used
     1 - alternate keyword color 1
     2 - alternate keyword color 2
     3 - alternate keyword color 3
     4 - alternate keyword color 4
     5 - alternate keyword color 5
     6 - alternate keyword color 6
     7 - alternate keyword color 7
     8 - alternate keyword color 8
     9 - alternate keyword color 9
     N/A indicates that this capability is not yet implemented.

     For valid values for 'modifier', 'foreground' and 'background'
     see <SET COLOR>.

     The second format of this command allows the user to turn on or off
     any of the valid modifiers.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     See <QUERY> ECOLOR

SEE ALSO
     <SET COLORING>, <SET AUTOCOLOR>, <SET PARSER>, <SET COLOR>
     Appendix 4

STATUS
     Complete.
**man-end**********************************************************************/
/*man-start*********************************************************************
COMMAND
     set ecolour - set colours for syntax highlighting

SYNTAX
     [SET] ECOLOUR char [modifier[...]] [foreground] [on background]
     [SET] ECOLOUR char [modifier[...]] ON|OFF

DESCRIPTION
     The SET ECOLOUR command allows the user to specify the colours of
     each category of items used in syntax highlighting.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     See <QUERY> ECOLOR

SEE ALSO
     <SET COLOURING>, <SET AUTOCOLOUR>, <SET PARSER>, <SET COLOUR>
     Appendix 4

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Ecolour(CHARTYPE *params)
#else
short Ecolour(params)
CHARTYPE *params;
#endif

/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define ECOL_PARAMS 2
   CHARTYPE *word[ECOL_PARAMS+1];
   CHARTYPE strip[ECOL_PARAMS];
   unsigned short num_params=0;
   short area=0,off;
   register short i=0;
   COLOUR_ATTR attr,tmp_attr;
   CHARTYPE *dummy=NULL;
   bool any_colours=FALSE;
   CHARTYPE ch;
   short word1_len,modifier_set=COL_MODIFIER_NO_SET;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Ecolour");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   num_params = param_split(params,word,ECOL_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 2 )
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Check which format of this command we are running.
    * If the last word is ON or OFF then we are executing the
    * second format.
    */
   word1_len = strlen( (DEFCHAR *)word[1] );
   if ( my_stricmp( (DEFCHAR *)word[1]+word1_len-2, "ON" ) == 0 )
      modifier_set = COL_MODIFIER_SET_ON;
   else if ( my_stricmp( (DEFCHAR *)word[1]+word1_len-3, "OFF" ) == 0 )
      modifier_set = COL_MODIFIER_SET_OFF;
   if ( modifier_set )
   {
      /*
       * Check that the supplied area matches one of the values in the area
       * array and that the length is at least as long as the minimum.
       */
      if (strlen((DEFCHAR *)word[0]) != 1)
      {
         display_error(1,word[0],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      ch = word[0][0];
      if (ch >= 'A' && ch <= 'Z')
         off = 'A';
      else if (ch >= 'a' && ch <= 'z')
         off = 'a';
      else if (ch >= '1' && ch <= '9')
         off = '1' - 26; /* Beware: --x == +x */
      else if (ch == '*' )
         off = -1;
      else
      {
         display_error(1,word[0],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      /*
       * Check that each subsequent parameter (except the last) is
       * a modifier.
       */
      if ( parse_modifiers( word[1], &tmp_attr ) != RC_OK )
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      if ( off == (-1) )
      {
         for ( i = 0; i < ECOLOUR_MAX; i++ )
         {
            attr = CURRENT_FILE->ecolour[i] ;
            if ( modifier_set == COL_MODIFIER_SET_ON )
            {
               if ( colour_support )
                  attr.mod |= tmp_attr.mod;
               else
                  attr.mono |= tmp_attr.mono;
            }
            else
            {
               if ( colour_support )
                  attr.mod &= ~tmp_attr.mod;
               else
                  attr.mono &= ~tmp_attr.mono;
            }
            CURRENT_FILE->ecolour[i] = attr;
         }
      }
      else
      {
         area = ch - off;
         attr = CURRENT_FILE->ecolour[area];
         if ( modifier_set == COL_MODIFIER_SET_ON )
         {
            if ( colour_support )
               attr.mod |= tmp_attr.mod;
            else
               attr.mono |= tmp_attr.mono;
         }
         else
         {
            if ( colour_support )
               attr.mod &= ~tmp_attr.mod;
            else
               attr.mono &= ~tmp_attr.mono;
         }
         CURRENT_FILE->ecolour[area] = attr;
      }
   }
   else
   {
      /*
       * Check that the supplied area matches one of the values in the area
       * array and that the length is at least as long as the minimum.
       */
      if (strlen((DEFCHAR *)word[0]) != 1)
      {
         display_error(1,word[0],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      ch = word[0][0];
      if (ch >= 'A' && ch <= 'Z')
         off = 'A';
      else if (ch >= 'a' && ch <= 'z')
         off = 'a';
      else if (ch >= '1' && ch <= '9')
         off = '1' - 26; /* Beware: --x == +x */
      else
      {
         display_error(1,word[0],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      area = ch - off;
      attr = CURRENT_FILE->ecolour[area];
      /*
       * Determine colours and modifiers.
       */
      if (parse_colours(word[1],&attr,&dummy,FALSE,&any_colours) != RC_OK)
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      /*
       * Now we have the new colours, save them with the current file...
       */
      CURRENT_FILE->ecolour[area] = attr;
   }
   /*
    * If we haven't started curses (in profile first time) exit now...
    */
   if (!curses_started)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * Update the appropriate window with the new colour combination...
    */
   if (display_screens > 1
   &&  SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen))
   {
      display_screen(other_screen);
   }
   display_screen(current_screen);
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set eolout - set end of line terminating character(s)

SYNTAX
     [SET] EOLout CRLF|LF|CR|NONE

DESCRIPTION
     The EOLOUT command allows the user to specify the combination of
     characters that terminate a line. Lines of text in Unix files are
     usually terminated with a 'LF', DOS file usually end with a 'CR' and
     'LF' combination. Files on the Apple Macintosh are usually terminated
     with a 'CR'.

     The 'NONE' option can be used to specify that no end of line
     character is written.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     LF - UNIX
     CRLF - DOS/OS2/WIN32
     NONE - if THE started with -u option

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Eolout(CHARTYPE *params)
#else
short Eolout(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 CHARTYPE eolchar=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Eolout");
 if (equal((CHARTYPE *)"lf",params,2))
    eolchar = EOLOUT_LF;
 else
    if (equal((CHARTYPE *)"cr",params,2))
       eolchar = EOLOUT_CR;
    else
      {
       if (equal((CHARTYPE *)"crlf",params,4))
          eolchar = EOLOUT_CRLF;
       else
          if (equal((CHARTYPE *)"none",params,4))
             eolchar = EOLOUT_NONE;
          else
            {
             display_error(1,(CHARTYPE *)params,FALSE);
             TRACE_RETURN();
             return(RC_INVALID_OPERAND);
            }
      }
 EOLx = CURRENT_FILE->eolout = eolchar;
 TRACE_RETURN();
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set equivchar - set the equivalence character

SYNTAX
     [SET] EQUIVChar char

DESCRIPTION
     The SET EQUIVChar command allows the user to change the character
     that is used to specify eqivalence in command parameters.

     In many THE commands, an equivalence character, usually '=', can
     be used as a parameter to default to values in the current file
     or view.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     =

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Equivchar(CHARTYPE *params)
#else
short Equivchar(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Equivchar");
   /*
    * Must supply a parameter...
    */
   if (blank_field(params))
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * ... and it must be a single character
    */
   if (strlen( (DEFCHAR *)params ) > 1 )
   {
      display_error(37,params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Save it.
    */
   EQUIVCHARstr[0] = EQUIVCHARx = *(params);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set etmode - indicate if extended display mode is possible

SYNTAX
     [SET] ETMODE ON|OFF [character list]

DESCRIPTION
     The SET ETMODE command allows the user to specify which characters
     in a character set are to be displayed as their actual representation.

     Those characters not explicitly specified to be displayed as they are
     represented, will be displayed as the <SET NONDISP> character in the
     colour specified by <SET COLOUR> NONDISP. Characters below 32, will
     be displayed with an alphabetic character representing the "control"
     code.

     eg.
     character code with a value of 7, will display as "G" in the colour
     specified by <SET COLOUR> NONDISP.

     'ON' with no optional 'character list' will display ALL
     characters as their actual representation.

     'OFF' with no optional 'character list' will display control
     characters below ASCII 32, as a "control" character; characters
     greater than ASCII 126 will be displayed as the <SET NONDISP>
     characters. On ASCII based machines, [SET] ETMODE OFF is
     equivalent  to [SET] ETMODE ON 32-126. On EBCDIC based machines
     [SET] ETMODE OFF is equivalent to [SET] ETMODE ON ??-??

     The 'character list' is a list of positive numbers between 0 and
     255 (inclusive).  The format of this character list can be either
     a single number; eg. 124, or a range of numbers specified; eg.
     32-126. (The first number must be less than or equal to the second
     number).

     As an example; ETMODE ON 32-127 160-250  would result in the
     characters with a decimal value between 32 and 127 inclusive
     and 160 and 250 inclusive being displayed as their actual
     representation (depending on the current font), and the
     characters between 0 and 31 inclusive, being displayed as
     an equivalent "control" character; characters between 128 and
     159 inculsive and 250 to 255 being displayed with the <SET NONDISP>
     character.

     Up to 20 character specifiers (single number or range) can be
     specified.

COMPATIBILITY
     XEDIT: Similar function but deals with Double-Byte characters
     KEDIT: N/A

DEFAULT
     ON - DOS/OS2/WIN32
     ON 32-255 - X11
     OFF - UNIX/AMIGA/QNX

SEE ALSO
     <SET NONDISP>, <SET COLOUR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Etmode(CHARTYPE *params)
#else
short Etmode(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define ETM_PARAMS  21
   CHARTYPE *word[ETM_PARAMS+1];
   CHARTYPE strip[ETM_PARAMS];
   short num_params=0;
   register short i=0,j=0;
   short rc=RC_OK;
   bool tmp_mode=FALSE;
   chtype attr=0L;
   COLOUR_ATTR curr_attr;
   bool flags[256];
   int num=0,num1=0;
   CHARTYPE *wptr=NULL,*wptr1=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Etmode");
   for(i=0;i<ETM_PARAMS;i++)
      strip[i]=STRIP_BOTH;
   num_params = param_split(params,word,ETM_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 1)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   rc = execute_set_on_off(word[0],&tmp_mode);
   if (rc != RC_OK)
   {
      display_error(1,word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (CURRENT_VIEW == NULL
   ||  CURRENT_FILE == NULL)
      set_up_default_colours((FILE_DETAILS *)NULL,&curr_attr,ATTR_NONDISP);
   else
      memcpy(&curr_attr,CURRENT_FILE->attr+ATTR_NONDISP,sizeof(COLOUR_ATTR));
   attr = set_colour(&curr_attr);
   if (num_params == 1)  /* absolute ON or OFF */
   {
      if (tmp_mode)  /* ETMODE ON */
      {
         for (i=0;i<256;i++)
         {
            etmode_table[i] = i;
            etmode_flag[i] = FALSE;
         }
      }
      else
      {
         for (i=0;i<256;i++)
         {
#if 1
            if (isprint(i) )
            {
               etmode_table[i] = i;
               etmode_flag[i] = FALSE;
            }
            else if (iscntrl(i) )
            {
               etmode_table[i] = ('@' + i) | attr;
               etmode_flag[i] = (attr)?TRUE:FALSE;
            }
            else
            {
               etmode_table[i] = NONDISPx | attr;
               etmode_flag[i] = (attr)?TRUE:FALSE;
            }
#else
            if (i < 32)
            {
               etmode_table[i] = ('@' + i) | attr;
               etmode_flag[i] = (attr)?TRUE:FALSE;
            }
            else if (i > 126)
            {
               etmode_table[i] = NONDISPx | attr;
               etmode_flag[i] = (attr)?TRUE:FALSE;
            }
            else
            {
               etmode_table[i] = i;
               etmode_flag[i] = FALSE;
            }
#endif
         }
      }
      if (number_of_files != 0)
      {
         build_screen(current_screen);
         display_screen(current_screen);
      }
      TRACE_RETURN();
      return(RC_OK);
   }
   memset(flags,FALSE,sizeof(flags));
   for (i=1;i<num_params;i++)
   {
      if (valid_positive_integer(word[i]))
      {
         num = atoi((DEFCHAR *)word[i]);
         if (num > 255)
         {
            display_error(6,word[i],FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
         }
         flags[num] = TRUE;
         continue;
      }
      num = strzeq(word[i],(CHARTYPE)'-');
      num1 = strzreveq(word[i],(CHARTYPE)'-');
      if (num != num1
      || num == (-1))
      {
         display_error(1,word[i],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      wptr = word[i];
      *(wptr+num) = '\0';
      wptr1 = wptr+num+1;
      if (!valid_positive_integer(wptr))
      {
         display_error(1,wptr,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      if (!valid_positive_integer(wptr))
      {
         display_error(1,wptr1,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      num = atoi((DEFCHAR *)wptr);
      num1 = atoi((DEFCHAR *)wptr1);
      if (num > num1)
      {
         display_error(1,word[i],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      if (num > 255)
      {
         display_error(6,wptr,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      if (num1 > 255)
      {
         display_error(6,wptr1,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      for (j=num;j<num1+1;j++)
      {
         flags[j] = TRUE;
      }
   }
   for (i=0;i<256;i++)
   {
      if (flags[i])
      {
         etmode_table[i] = i;
         etmode_flag[i] = FALSE;
      }
      else
      {
#if 1
         if (iscntrl(i) )
#else
         if (i < 32)
#endif
         {
            etmode_table[i] = ('@' + i) | attr;
            etmode_flag[i] = TRUE;
         }
         else
         {
            etmode_table[i] = NONDISPx | attr;
            etmode_flag[i] = TRUE;
         }
      }
   }
   if (number_of_files != 0)
   {
      build_screen(current_screen);
      display_screen(current_screen);
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set fext - change the extension of the existing file

SYNTAX
     [SET] FExt ext
     [SET] FType ext

DESCRIPTION
     The SET FEXT command allows the user to change the path of
     the file currently being edited.

     The 'path' parameter can be specified with or without the
     trailing directory seperator.  Under DOS, OS/2 and Windows ports,
     the drive letter is considered part of the file's path.

     See <SET FILENAME> for a full explanation of THE's definitions
     of fpath, filename, fname, fext and fmode.

     It is not possible to use this command on pseudo files.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SET FNAME>, <SET FILENAME>, <SET FTYPE>, <SET FMODE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Fext(CHARTYPE *params)
#else
short Fext(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 CHARTYPE tmp_name[MAX_FILE_NAME+1];
 short rc=RC_OK;
 int last_period=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Fext");
/*
 * If a pseudo file is being changed, then error...
 */
 if (CURRENT_FILE->pseudo_file)
   {
    display_error(8,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 strcpy((DEFCHAR *)tmp_name,(DEFCHAR *)CURRENT_FILE->fpath);
 strcat((DEFCHAR*)tmp_name,(DEFCHAR*)CURRENT_FILE->fname);
 last_period = strzreveq(CURRENT_FILE->fname,(CHARTYPE)'.');
 if (last_period == (-1)) /* no period */
   {
    if (blank_field(params)) /* and no extension, return... */
      {
       TRACE_RETURN();
       return(RC_OK);
      }
    strcat((DEFCHAR*)tmp_name,"."); /* add a period */
   }
 else
   {
    tmp_name[strlen((DEFCHAR*)CURRENT_FILE->fpath)+last_period+1] = '\0';
   }
 strcat((DEFCHAR*)tmp_name,(DEFCHAR*)params);
/*
 * Split the new path supplied...
 */
 if ((rc = splitpath(strrmdup(strtrans(tmp_name,OSLASH,ISLASH),ISLASH,TRUE))) != RC_OK)
   {
    display_error(10,tmp_name,FALSE);
    TRACE_RETURN();
    return(rc);
   }
/*
 * If the path is NOT the same as already assigned, error...
 */
 if (strcmp((DEFCHAR *)sp_path,(DEFCHAR *)CURRENT_FILE->fpath) != 0)
   {
    display_error(1,params,FALSE);
    TRACE_RETURN();
    return(RC_OK);
   }
/*
 * If the length of the new path is > the existing one,
 * free up any memory for the existing path and allocate some
 * more. Save the new path.
 */
 if (strlen((DEFCHAR*)sp_fname) > strlen((DEFCHAR*)CURRENT_FILE->fname))
   {
    (*the_free)(CURRENT_FILE->fname);
    if ((CURRENT_FILE->fname = (CHARTYPE *)(*the_malloc)(strlen((DEFCHAR *)sp_fname))) == NULL)
      {
       display_error(30,(CHARTYPE *)"",FALSE);
       TRACE_RETURN();
       return(RC_OUT_OF_MEMORY);
      }
   }
 strcpy((DEFCHAR *)CURRENT_FILE->fname,(DEFCHAR *)sp_fname);
/*
 * Re-display the IDLINE
 */
 if (display_screens > 1
 &&  SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen))
    show_heading(other_screen);
 show_heading(current_screen);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set filename - change the filename of the file being edited

SYNTAX
     [SET] FILEName filename

DESCRIPTION
     The SET FILEName command allows the user to change the filename of
     the file currently being edited.

     In THE, a fully qualified file name consists of a file path and a
     file name.  THE treats all characters up to and including the
     last directory seperator (usually / or \) as the file's path.
     From the first character after the end of the file's path, to
     the end of the fully qualified file name is the file name.

     A file name is further broken down into a fname and fext.
     The fname of a file consists of all characters from the start
     of the filename up to but not including the last period (if
     there is one).  The fext of a file consists of all characters
     from the end of the filename up to but not including the last
     period. If there is no period in the filename then the fext is
     empty.

     The fmode of a file is equivalent to the drive letter of the file's
     path. This is only valid under DOS, OS/2 and Windows ports.

     Some examples.

     *----------------------------------------------------------------
     Full File Name     File            File     Fname  Fext     Fmode
                        Path            Name
     -----------------------------------------------------------------
     /usr/local/bin/the /usr/local/bin/ the      the             N/A
     c:\tools\the.exe   c:\tools\       the.exe  the    exe      c
     /etc/a.b.c         /etc/           a.b.c    a.b    c        N/A
     *----------------------------------------------------------------

     A limited amount of validation of the resulting file name is
     carried out by this command, but some errors in the file name
     will not be evident until the file is saved.

     A leading "=" indicates that the fname portion of the current file
     name is be retained.  This is equivalent to the command
     <SET FEXT>.  A trailing "=" indicates that the fext portion of
     the current file name is to be retained. This is equivalent to the
     command <SET FNAME>.

     Only one "=" is allowed in the parameter.

     Some examples.

     *----------------------------------------------------------------
     File Name   Parameter  New File Name
     -----------------------------------------------------------------
     a.b.c       fred.c=    fred.c.c      SET FNAME fred.c
     a.b.c       fred.c.=   fred.c..c     SET FNAME fred.c.
     a.b.c       =fred      a.c.fred      SET FEXT fred
     a.b.c       =.fred     a.c..fred     SET FEXT .fred
     a           =d         a.d           SET FEXT d
     a.b.c       =          a.b.c         does nothing
     *----------------------------------------------------------------

     It is not possible to use this command on pseudo files.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SET FPATH>, <SET FNAME>, <SET FEXT>, <SET FMODE>, <SET EQUIVCHAR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Filename(CHARTYPE *params)
#else
short Filename(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   CHARTYPE tmp_name[MAX_FILE_NAME+1];
   short rc=RC_OK;
   int i=0,cnt=0,len_params=0;

   TRACE_FUNCTION("commset1.c:Filename");
  /*
   * Must supply a parameter...
   */
   if (blank_field(params))
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If a pseudo file is being changed, then error...
    */
   if (CURRENT_FILE->pseudo_file)
   {
      display_error(8,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If a = is specified...
    */
   if (strcmp((DEFCHAR*)EQUIVCHARstr,(DEFCHAR*)params) == 0)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * Find out how many = are specified...
    */
   len_params = strlen((DEFCHAR*)params);
   for (i=0,cnt=0;i<len_params;i++)
   {
      if (params[i] == EQUIVCHARx)
         cnt++;
   }
   if (cnt > 1)
   {
      display_error(1,params,FALSE);
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * If we do have a leading or trailing = then call the equivalent
    * SET FEXT or FNAME command...
    */
   if (cnt == 1)
   {
      if (params[0] == EQUIVCHARx)
      {
         strcpy((DEFCHAR*)tmp_name,(DEFCHAR*)params+1);
         rc = Fext(tmp_name);
         TRACE_RETURN();
         return(rc);
      }
      else
      {
         if (params[len_params-1] == EQUIVCHARx)
         {
            strcpy((DEFCHAR*)tmp_name,(DEFCHAR*)params);
            tmp_name[len_params-1] = '\0';
            rc = Fname(tmp_name);
            TRACE_RETURN();
            return(rc);
         }
         else
         {
            display_error(1,params,FALSE);
            TRACE_RETURN();
            return(rc);
         }
      }
   }
   /*
    * To get here, no = was in the parameter...
    */
   strcpy((DEFCHAR *)tmp_name,(DEFCHAR *)CURRENT_FILE->fpath);
   strcat((DEFCHAR *)tmp_name,(DEFCHAR *)params);
   if ((rc = splitpath(strrmdup(strtrans(tmp_name,OSLASH,ISLASH),ISLASH,TRUE))) != RC_OK)
   {
      display_error(10,tmp_name,FALSE);
      TRACE_RETURN();
      return(rc);
   }
   /*
    * If the resulting path is different to the current one, error.
    */
   if (strcmp((DEFCHAR *)sp_path,(DEFCHAR *)CURRENT_FILE->fpath) != 0)
   {
      display_error(8,params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If the file name is the same as already assigned, exit...
    */
   if (strcmp((DEFCHAR *)sp_fname,(DEFCHAR *)CURRENT_FILE->fname) == 0)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * If the length of the new filename is > the existing one,
    * free up any memory for the existing name and allocate some
    * more. Save the new name.
    */
   if (strlen((DEFCHAR*)sp_fname) > strlen((DEFCHAR*)CURRENT_FILE->fname))
   {
      (*the_free)(CURRENT_FILE->fname);
      if ((CURRENT_FILE->fname = (CHARTYPE *)(*the_malloc)(strlen((DEFCHAR *)sp_fname)+1)) == NULL)
      {
         display_error(30,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_OUT_OF_MEMORY);
      }
   }
   strcpy((DEFCHAR *)CURRENT_FILE->fname,(DEFCHAR *)sp_fname);
   /*
    * Re-display the IDLINE
    */
   if (display_screens > 1
   &&  SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen))
      show_heading(other_screen);
   show_heading(current_screen);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set filetabs - determine if and where where file tabs are positioned

SYNTAX
     [SET] FILETABS ON|OFF

DESCRIPTION
     The SET FILETABS command allows the user to determine if file tabs
     are to be displayed and where. FILETABS is a single line at the
     top of the display showing all files currently in the ring, except
     the current file.
     It provides a mechanism where the user running THE with mouse support
     can simply click on the filename in the FILETABS line to change focus
     to that file.

     The colour of the file tabs can be set with <SET COLOUR> FILETABS.
     The colour of the file dividers can be set with <SET COLOUR> FILETABSDIV.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SET COLOUR>, <TABFILE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Filetabs(CHARTYPE *params)
#else
short Filetabs(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   bool save_filetabs=FILETABSx;

   TRACE_FUNCTION("commset1.c:Filetabs");
   rc = execute_set_on_off( params, &FILETABSx );
   if ( save_filetabs != FILETABSx )
   {
      /*
       * To get here something has changed, so rebuild the windows and
       * display the screen.
       */
      set_screen_defaults();
      if ( set_up_windows( current_screen ) != RC_OK )
      {
         TRACE_RETURN();
         return(RC_OK);
      }
      if ( display_screens > 1
      &&  SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen) )
      {
         build_screen( other_screen );
         if ( curses_started )
            display_screen( other_screen );
      }
      build_screen( current_screen );
      if ( curses_started )
         display_screen( current_screen );
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set fmode - change the drive letter of the existing file

SYNTAX
     [SET] FMode d[:]

DESCRIPTION
     The SET FMode command allows the user to change the drive letter
     of the file currently being edited.

     This command is only valid under the DOS, OS/2 and Windows ports.

     See <SET FILENAME> for a full explanation of THE's definitions
     of fpath, filename, fname, fext and fmode.

     It is not possible to use this command on pseudo files.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible

SEE ALSO
     <SET FNAME>, <SET FILENAME>, <SET FEXT>, <SET FPATH>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Fmode(CHARTYPE *params)
#else
short Fmode(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#ifndef UNIX
   CHARTYPE tmp_name[MAX_FILE_NAME+1];
   int len_params=0;
#endif
   short rc=RC_OK;

   TRACE_FUNCTION("commset1.c:Fmode");
   /*
    * Not valid for Unix...
    */
#ifdef UNIX
   display_error( 82, (CHARTYPE *)"FMODE", FALSE );
   rc = RC_INVALID_OPERAND;
#else
   /*
    * Must supply a parameter...
    */
   if (blank_field(params))
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If a pseudo file is being changed, then error...
    */
   if (CURRENT_FILE->pseudo_file)
   {
    display_error(8,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
   /*
    * The only valid parameter is an alphabetic with an optional
    * ':'
    */
   len_params = strlen((DEFCHAR*)params);
   if ( len_params > 2
   ||   !isalpha(*params)
   ||   (len_params == 2
      && *params != ':') )
   {
      display_error(1,params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   strcpy((DEFCHAR*)tmp_name,(DEFCHAR*)CURRENT_FILE->fpath);
   memcpy((DEFCHAR*)tmp_name,(DEFCHAR*)params,len_params);
   /*
    * Split the new path supplied...
    */
   if ((rc = splitpath(strrmdup(strtrans(tmp_name,OSLASH,ISLASH),ISLASH,TRUE))) != RC_OK)
   {
      display_error(10,tmp_name,FALSE);
      TRACE_RETURN();
      return(rc);
   }
   /*
    * If a filename results, then the path name specified would conflict
    * with an existing file.
    */
   if (!blank_field(sp_fname))
   {
      display_error(8,params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If the path is the same as already assigned, exit...
    */
   if (strcmp((DEFCHAR *)sp_path,(DEFCHAR *)CURRENT_FILE->fpath) == 0)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * If the length of the new path is > the existing one,
    * free up any memory for the existing path and allocate some
    * more. Save the new path.
    */
   if (strlen((DEFCHAR*)sp_path) > strlen((DEFCHAR*)CURRENT_FILE->fpath))
   {
      (*the_free)(CURRENT_FILE->fpath);
      if ((CURRENT_FILE->fpath = (CHARTYPE *)(*the_malloc)(strlen((DEFCHAR *)sp_path))) == NULL)
      {
         display_error(30,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_OUT_OF_MEMORY);
      }
   }
   strcpy((DEFCHAR *)CURRENT_FILE->fpath,(DEFCHAR *)sp_path);
   /*
    * Re-display the IDLINE
    */
   if (display_screens > 1
   &&  SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen))
      show_heading(other_screen);
   show_heading(current_screen);
#endif
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set fname - change the filename of the file being edited

SYNTAX
     [SET] FName filename

DESCRIPTION
     The SET FNAME command allows the user to change the fname of
     the file currently being edited.

     See <SET FILENAME> for a full explanation of THE's definitions
     of fpath, filename, fname, fext and fmode.

     A limited amount of validation of the resulting file name is
     carried out by this command, but some errors in the file name
     will not be evident until the file is saved.

     It is not possible to use this command on pseudo files.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SET FPATH>, <SET FILENAME>, <SET FEXT>, <SET FMODE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Fname(CHARTYPE *params)
#else
short Fname(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   CHARTYPE tmp_name[MAX_FILE_NAME+1];
   short rc=RC_OK;
   int last_period=0;

   TRACE_FUNCTION( "commset1.c:Fname" );
   /*
    * Must supply a parameter...
    */
   if ( blank_field( params ) )
   {
      display_error( 3, (CHARTYPE *)"", FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If a pseudo file is being changed, then error...
    */
   if ( CURRENT_FILE->pseudo_file )
   {
      display_error( 8, (CHARTYPE *)"", FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   strcpy( (DEFCHAR *)tmp_name, (DEFCHAR *)CURRENT_FILE->fpath );
   last_period = strzreveq( CURRENT_FILE->fname, (CHARTYPE)'.' );
   if ( last_period == (-1) ) /* no period */
   {
      strcat( (DEFCHAR*)tmp_name, (DEFCHAR*)params );
   }
   else
   {
      int len=strlen( (DEFCHAR*)CURRENT_FILE->fpath );
      int lenext=strlen( (DEFCHAR*)CURRENT_FILE->fname) - last_period;
      strcat( (DEFCHAR*)tmp_name, (DEFCHAR*)CURRENT_FILE->fname + last_period );
      meminsmem( tmp_name, params, strlen( (DEFCHAR*)params), len, MAX_FILE_NAME + 1, len + lenext + 1 );
   }
   /*
    * Split the new path supplied...
    */
   if ( ( rc = splitpath( strrmdup( strtrans( tmp_name, OSLASH, ISLASH ), ISLASH, TRUE ) ) ) != RC_OK )
   {
      display_error( 10, tmp_name, FALSE );
      TRACE_RETURN();
      return(rc);
   }
   /*
    * If the resulting path is different to the current one, error.
    */
   if ( strcmp( (DEFCHAR *)sp_path, (DEFCHAR *)CURRENT_FILE->fpath ) != 0 )
   {
      display_error( 8, params, FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If the file name is the same as already assigned, exit...
    */
   if ( strcmp( (DEFCHAR *)sp_fname, (DEFCHAR *)CURRENT_FILE->fname ) == 0 )
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * If the length of the new path is > the existing one,
    * free up any memory for the existing path and allocate some
    * more. Save the new path.
    */
   if ( strlen( (DEFCHAR*)sp_fname ) > strlen( (DEFCHAR*)CURRENT_FILE->fname ) )
   {
      (*the_free)( CURRENT_FILE->fname );
      if ( ( CURRENT_FILE->fname = (CHARTYPE *)(*the_malloc)( strlen( (DEFCHAR *)sp_fname ) ) ) == NULL )
      {
         display_error( 30, (CHARTYPE *)"", FALSE );
         TRACE_RETURN();
         return(RC_OUT_OF_MEMORY);
      }
   }
   strcpy( (DEFCHAR *)CURRENT_FILE->fname, (DEFCHAR *)sp_fname );
   /*
    * Re-display the IDLINE
    */
    if ( display_screens > 1
    &&   SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen) )
       show_heading( other_screen );
    show_heading( current_screen );
    TRACE_RETURN();
    return(rc);
}
/*man-start*********************************************************************
COMMAND
     set fpath - change the path of the existing file

SYNTAX
     [SET] FPath path

DESCRIPTION
     The SET FPATH command allows the user to change the path of
     the file currently being edited.

     The 'path' parameter can be specified with or without the
     trailing directory seperator.  Under DOS, OS/2 and Windows ports,
     the drive letter is considered part of the file's path.

     See <SET FILENAME> for a full explanation of THE's definitions
     of fpath, filename, fname, fext and fmode.

     It is not possible to use this command on pseudo files.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SET FNAME>, <SET FILENAME>, <SET FEXT>, <SET FMODE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Fpath(CHARTYPE *params)
#else
short Fpath(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Fpath");
/*
 * Must supply a parameter...
 */
 if (blank_field(params))
   {
    display_error(3,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
/*
 * If a pseudo file is being changed, then error...
 */
 if (CURRENT_FILE->pseudo_file)
   {
    display_error(8,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
/*
 * Split the new path supplied...
 */
 if ((rc = splitpath(strrmdup(strtrans(params,OSLASH,ISLASH),ISLASH,TRUE))) != RC_OK)
   {
    display_error(10,params,FALSE);
    TRACE_RETURN();
    return(rc);
   }
/*
 * If a filename results, then the path name specified would conflict
 * with an existing file.
 */
 if (!blank_field(sp_fname))
   {
    display_error(8,params,FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
/*
 * If the path is the same as already assigned, exit...
 */
 if (strcmp((DEFCHAR *)sp_path,(DEFCHAR *)CURRENT_FILE->fpath) == 0)
   {
    TRACE_RETURN();
    return(RC_OK);
   }
/*
 * If the length of the new path is > the existing one,
 * free up any memory for the existing path and allocate some
 * more. Save the new path.
 */
 if (strlen((DEFCHAR*)sp_path) > strlen((DEFCHAR*)CURRENT_FILE->fpath))
   {
    (*the_free)(CURRENT_FILE->fpath);
    if ((CURRENT_FILE->fpath = (CHARTYPE *)(*the_malloc)(strlen((DEFCHAR *)sp_path))) == NULL)
      {
       display_error(30,(CHARTYPE *)"",FALSE);
       TRACE_RETURN();
       return(RC_OUT_OF_MEMORY);
      }
   }
 strcpy((DEFCHAR *)CURRENT_FILE->fpath,(DEFCHAR *)sp_path);
/*
 * Re-display the IDLINE
 */
 if (display_screens > 1
 &&  SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen))
    show_heading(other_screen);
 show_heading(current_screen);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set ftype - change the extension of the existing file

SYNTAX
     [SET] FType ext

DESCRIPTION
     The SET FTYPE is a synonym for <SET FEXT>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SET FNAME>, <SET FILENAME>, <SET FEXT>, <SET FMODE>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     set fullfname - specify if complete filename to be displayed

SYNTAX
     [SET] FULLFName ON|OFF

DESCRIPTION
     The SET FULLFNAME command allows the user to determine if the
     fully qualified filename is displayed on the IDLINE or just the
     FNAME component.
     See <SET FILENAME> for a full explanation of THE's definitions
     of fpath, filename, fname, fext and fmode.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     ON

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Fullfname(CHARTYPE *params)
#else
short Fullfname(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
    short rc=RC_OK;

    TRACE_FUNCTION("commset1.c:Fullfname");
    rc = execute_set_on_off(params,&CURRENT_FILE->display_actual_filename);
    if (display_screens > 1
    &&  SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen))
       show_heading(other_screen);
    show_heading(current_screen);
    TRACE_RETURN();
    return(rc);
}
/*man-start*********************************************************************
COMMAND
     set header - turn on or off syntax highlighting headers

SYNTAX
     [SET] HEADer section ON|OFF

DESCRIPTION
     The SET HEADER command allows fine tuning of which sections of a
     TLD file are to be applied for the current view.

     'section' refers to one of the following headers that can be specified
     in a TLD file:
     NUMBER, COMMENT, STRING, KEYWORD, FUNCTION, HEADER, LABEL, MATCH,
     COLUMN, POSTCOMPARE, MARKUP.
     'section' can also be specified as '*', in which case, all headers
     are applied or not applied.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     * ON

SEE ALSO
     <SET PARSER>, <SET COLORING>, <SET AUTOCOLOR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THEHeader(CHARTYPE *params)
#else
short THEHeader(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
#define HEA_PARAMS  2
   CHARTYPE *word[HEA_PARAMS+1];
   CHARTYPE strip[HEA_PARAMS];
   short num_params=0;
   LINETYPE save_syntax_headers=CURRENT_VIEW->syntax_headers,val=0;
   bool on_or_off;
   int i;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:THEHeader");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   num_params = param_split(params,word,HEA_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 1)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Validate first parameter...
    */
   for ( i = 0; thm[i].the_header_name != NULL; i++ )
   {
      if (equal((CHARTYPE *)thm[i].the_header_name,word[0],thm[i].the_header_name_len))
      {
         val = thm[i].the_header;
         break;
      }
   }
   if ( val == 0 )
   {
      display_error(1,word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }

   rc = execute_set_on_off(word[1],&on_or_off);
   if ( rc != RC_OK )
   {
      TRACE_RETURN();
      return(rc);
   }

   /*
    * Have a valid header, and a valid ON|OFF...
    */
   if ( on_or_off )
      CURRENT_VIEW->syntax_headers |= val;
   else
      CURRENT_VIEW->syntax_headers &= ~val;

   if ( CURRENT_VIEW->syntax_headers != save_syntax_headers )
   {
      build_screen(current_screen);
      display_screen(current_screen);
   }

   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set hex - set how hexadecimal strings are treated in string operands

SYNTAX
     [SET] HEX ON|OFF

DESCRIPTION
     The SET HEX set command determines whether hexadecimal strings are
     treated as such in string operands.

     With the 'ON' option, any string operand of the form
        /x'31 32 33'/ or
        /d'49 50 51'/
     will be converted to /123/ before the command is executed.

     With the 'OFF' option, no conversion is done.

     This conversion should work wherever a string operand is used
     in any command.

COMPATIBILITY
     XEDIT: Adds support for decimal representation. See below.
     KEDIT: Compatible. See below.
     Spaces must seperate each character representation.

DEFAULT
     OFF

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Hex(CHARTYPE *params)
#else
short Hex(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Hex");
 rc = execute_set_on_off(params,&CURRENT_VIEW->hex);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set hexdisplay - turn on or off display of character under cursor

SYNTAX
     [SET] HEXDISPlay ON|OFF

DESCRIPTION
     The SET HEXDISPLAY command turns on or off the display of the
     character under the cursor on the <status line>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     ON

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Hexdisplay(CHARTYPE *params)
#else
short Hexdisplay(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Hexdisplay");
 rc = execute_set_on_off(params,&HEXDISPLAYx);
 if (rc == RC_OK
 &&  curses_started)
    clear_statarea();
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set hexshow - turn on or off hex display of current line

SYNTAX
     [SET] HEXShow ON|OFF [M[+n|-n]|[+|-]n]

DESCRIPTION
     The SET HEXShow command indicates if and where a hexadecimal
     representation of the <current line> will be displayed.

     The first form of parameters is:

     M[+n|-n]
     this sets the hexshow line to be relative to the middle of
     the screen. A positive value adds to the middle line number,
     a negative subtracts from it.
     eg. M+3 on a 24 line screen will be line 15
         M-5 on a 24 line screen will be line 7

     The second form of parameters is:

     [+|-]n
     this sets the hexshow line to be relative to the top of the
     screen (if positive or no sign) or relative to the bottom
     of the screen if negative.
     eg. +3 or 3 will set current line to line 3
         -3 on a 24 line screen will be line 21

     If the resulting line is outside the bounds of the screen
     the position of the hexshow line will become the middle line
     on the screen.

     The position argument specifies the position of the first line
     of the hexadecimal display.

     It is an error to try to position the HEXSHOW lines on the same
     line as <SET CURLINE>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     OFF 7

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Hexshow(CHARTYPE *params)
#else
short Hexshow(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define HEXS_PARAMS  2
 CHARTYPE *word[HEXS_PARAMS+1];
 CHARTYPE strip[HEXS_PARAMS];
 short num_params=0;
 short rc=RC_OK;
 short base=(short)CURRENT_VIEW->hexshow_base;
 short off=CURRENT_VIEW->hexshow_off;
 bool hexshowsts=FALSE;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Hexshow");
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,HEXS_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Parse the status parameter...                                       */
/*---------------------------------------------------------------------*/
 rc = execute_set_on_off(word[0],&hexshowsts);
 if (rc != RC_OK)
   {
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* Parse the position parameter...                                     */
/*---------------------------------------------------------------------*/
 if (num_params > 1)
   {
    rc = execute_set_row_position(word[1],&base,&off);
    if (rc != RC_OK)
      {
       TRACE_RETURN();
       return(rc);
      }
   }
/*---------------------------------------------------------------------*/
/* If the HEXSHOW row (or the next row) is the same row as CURLINE and */
/* it is being turned on, return ERROR.                                */
/*---------------------------------------------------------------------*/
 if ((calculate_actual_row(CURRENT_VIEW->current_base,
                          CURRENT_VIEW->current_off,
                          CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE) ==
     calculate_actual_row(base,off,CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE)
     || calculate_actual_row(CURRENT_VIEW->current_base,
                             CURRENT_VIEW->current_off,
                             CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE) ==
        calculate_actual_row(base,off,CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE) + 1)
 && hexshowsts)
   {
    display_error(64,(CHARTYPE *)"- same line as CURLINE",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 CURRENT_VIEW->hexshow_base = (CHARTYPE)base;
 CURRENT_VIEW->hexshow_off = off;
 CURRENT_VIEW->hexshow_on = hexshowsts;
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
 build_screen(current_screen);
 display_screen(current_screen);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set highlight - specify which lines (if any) are to be highlighted

SYNTAX
     [SET] HIGHlight OFF|TAGged|ALTered|SELect n [m]

DESCRIPTION
     The SET HIGHLIGHT command allows for the user to specify which
     lines are to be displayed in the highlighted colour.

     'OFF' turns all highlighting display off

     'TAGGED' displays all tagged lines in the highlight colour.

     'ALTERED' displays all lines that have been added or
     changed in the current session in the highlight colour.

     'SELECT n [m]' displays all lines with the specified selection
     level in highlight colour.


COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible

DEFAULT
     OFF

SEE ALSO
     <SET SELECT>, <TAG>, <SET LINEFLAG>

STATUS
     Ccomplete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Highlight(CHARTYPE *params)
#else
short Highlight(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define HIGH_PARAMS  2
 CHARTYPE *word[HIGH_PARAMS+1];
 CHARTYPE strip[HIGH_PARAMS];
 short num_params=0;
 short col1=0,col2=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Highlight");
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,HIGH_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 switch (num_params)
   {
    case 1:
         if (equal((CHARTYPE *)"off",word[0],3))
           {
            CURRENT_VIEW->highlight = HIGHLIGHT_NONE;
            break;
           }
         if (equal((CHARTYPE *)"tagged",word[0],3))
           {
            CURRENT_VIEW->highlight = HIGHLIGHT_TAG;
            break;
           }
         if (equal((CHARTYPE *)"altered",word[0],3))
           {
            CURRENT_VIEW->highlight = HIGHLIGHT_ALT;
            break;
           }
         display_error(1,word[0],FALSE);
         rc = RC_INVALID_OPERAND;
         break;
    case 2:
    case 3:
         if (!equal((CHARTYPE *)"select",word[0],3))
           {
            display_error(1,word[0],FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
           }
         if ((rc = validate_n_m(word[1],&col1,&col2)) != RC_OK)
           {
            TRACE_RETURN();
            return(rc);
           }
         CURRENT_VIEW->highlight = HIGHLIGHT_SELECT;
         CURRENT_VIEW->highlight_low = col1;
         CURRENT_VIEW->highlight_high = col2;
         break;
    default:
         display_error(1,word[0],FALSE);
         rc = RC_INVALID_OPERAND;
         break;
  }
 if (rc == RC_OK)
   {
    build_screen(current_screen);
    display_screen(current_screen);
   }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set idline - specify if IDLINE is displayed

SYNTAX
     [SET] IDline ON|OFF

DESCRIPTION
     The SET IDLINE set command determines if the <idline> for a file is
     displayed or not.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     ON

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Idline(CHARTYPE *params)
#else
short Idline(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 bool save_id_line=FALSE;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Idline");
 save_id_line = CURRENT_VIEW->id_line;
 rc = execute_set_on_off(params,&CURRENT_VIEW->id_line);
 if (rc != RC_OK)
   {
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* If the new value of id_line is the same as before, exit now.        */
/*---------------------------------------------------------------------*/
 if (save_id_line == CURRENT_VIEW->id_line)
   {
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* Redefine the screen sizes...                                        */
/*---------------------------------------------------------------------*/
 set_screen_defaults();
/*---------------------------------------------------------------------*/
/* Recreate windows for the current screen...                          */
/*---------------------------------------------------------------------*/
 if (curses_started)
   {
    if (set_up_windows(current_screen) != RC_OK)
      {
       TRACE_RETURN();
       return(rc);
      }
   }
 build_screen(current_screen);
 display_screen(current_screen);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set impcmscp - set implied operating system command processing

SYNTAX
     [SET] IMPcmscp ON|OFF

DESCRIPTION
     The SET IMPCMSCP command is used to toggle implied operating system
     command processing from the command line. By turning this feature
     on you can then issue an operating system command without the need
     to prefix the operating system command with the <OS> command.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: N/A

DEFAULT
     ON

SEE ALSO
     <SET IMPOS>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     set impmacro - set implied macro command processing

SYNTAX
     [SET] IMPMACro ON|OFF

DESCRIPTION
     The SET IMPMACRO command is used to toggle implied macro processing
     from the command line. By turning this feature on you can then
     issue a <macro> command without the need to prefix the macro name
     with the <MACRO> command.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     ON

SEE ALSO
     <MACRO>, <SET MACROPATH>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Impmacro(CHARTYPE *params)
#else
short Impmacro(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Impmacro");
 rc = execute_set_on_off(params,&CURRENT_VIEW->imp_macro);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set impos - set implied operating system command processing

SYNTAX
     [SET] IMPOS ON|OFF

DESCRIPTION
     The SET IMPOS command is used to toggle implied operating system
     command processing from the command line. By turning this feature
     on you can then issue an operating system command without the need
     to prefix the operating system command with the <OS> command.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: N/A

DEFAULT
     ON

SEE ALSO
     <SET IMPCMSCP>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Impos(CHARTYPE *params)
#else
short Impos(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Impos");
 rc = execute_set_on_off(params,&CURRENT_VIEW->imp_os);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set inputmode - set input mode behaviour

SYNTAX
     [SET] INPUTMode OFF|FUll|LIne

DESCRIPTION
     The SET INPUTMODE command changes the way THE handles input.

     When INPUTMODE LINE is in effect, pressing the ENTER key while
     in the <filearea> will result in a new line being added.

     When INPUTMODE OFF is in effect, pressing the ENTER key while
     in the <filearea> will result in the cursor moving to the
     beginning of the next line; scrolling the screen if necessary.

     When INPUTMODE FULL is in effect, pressing the ENTER key while
     in the <filearea> will result in the cursor moving to the
     beginning of the next line; scrolling the screen if necessary.


COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     LINE

SEE ALSO
     <INPUT>

STATUS
     Incomplete. No support for FULL option.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Inputmode(CHARTYPE *params)
#else
short Inputmode(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Inputmode");
 if (equal((CHARTYPE *)"off",params,3))
    CURRENT_VIEW->inputmode = INPUTMODE_OFF;
 else
    if (equal((CHARTYPE *)"full",params,2))
       CURRENT_VIEW->inputmode = INPUTMODE_FULL;
    else
       if (equal((CHARTYPE *)"line",params,2))
          CURRENT_VIEW->inputmode = INPUTMODE_LINE;
       else
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
     set insertmode - put editor into or out of insert mode

SYNTAX
     [SET] INSERTMode ON|OFF|TOGGLE

DESCRIPTION
     The SET INSERTMODE command enable the user to set the insert mode
     within THE.

     The 'TOGGLE' option turns insert mode 'ON' if it is currently
     'OFF' and vice versa.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     OFF

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Insertmode(CHARTYPE *params)
#else
short Insertmode(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Insertmode");
 if (equal((CHARTYPE *)"off",params,3))
    INSERTMODEx = FALSE;
 else
    if (equal((CHARTYPE *)"on",params,2))
       INSERTMODEx = TRUE;
    else
       if (equal((CHARTYPE *)"toggle",params,6))
          INSERTMODEx = (INSERTMODEx) ? FALSE : TRUE;
       else
         {
          display_error(1,(CHARTYPE *)params,FALSE);
          TRACE_RETURN();
          return(RC_INVALID_OPERAND);
         }
 if (curses_started)
    draw_cursor(TRUE);
 TRACE_RETURN();
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set interface - set overall behaviour of THE

SYNTAX
     [SET] INTerface CLASSIC|CUA

DESCRIPTION
     The SET INTERFACE command changes the behaviour of several operations
     within THE.  THE normally operates in a block-mode manner, however
     many applications conform to the Common User Access (CUA) standard
     developed by IBM.  This command specifies that CUA behaiour should
     occur on various actions during the edit session.

     The major differences between CLASSIC and CUA behaviour, involve
     keyboard and mouse actions. Various THE commands have CUA options to
     allow the user to customise the behaviour individual keys or the
     mouse to behave in a CUA manner.

     Where behaviour is not related to particular key or mouse actions,
     this command provides the mechanism for changing the behaviour.
     The behaviour that SET INTERFACE affects:

     - entering text in the filearea with a marked CUA block will
       first delete the block and reposition the cursor
     - executing <SOS DELCHAR> or <SOS DELBACK> will delete the
       marked CUA block
     - executing any positioning command, such as <CURSOR> DOWN,
       <FORWARD> or <CURSOR> MOUSE, will unmark the CUA block

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible with KEDIT for Windows.

DEFAULT
     CLASSIC

SEE ALSO
     <MARK> <CURSOR>

STATUS
     Incomplete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THEInterface(CHARTYPE *params)
#else
short THEInterface(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:THEInterface");

   if ( equal( (CHARTYPE *)"classic", params, 7 ) )
      INTERFACEx = INTERFACE_CLASSIC;
   else if ( equal( (CHARTYPE *)"cua", params, 3 ) )
      INTERFACEx = INTERFACE_CUA;
   else
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      rc = RC_INVALID_OPERAND;
   }
   TRACE_RETURN();
   return(rc);
}

/*man-start*********************************************************************
COMMAND
     set lineflag - set the line characteristics of lines

SYNTAX
     [SET] LINEFLAG CHAnge|NOCHange NEW|NONEW TAG|NOTAG [target]

DESCRIPTION
     The SET LINEFLAGS command controls the line characteristics of lines
     in a file.

     Each line in a file has certain characteristics associated with it
     depending on how the line has been modified.  On reading a file from
     disk, all lines in the file are set to their default values.

     Once a line is modified, or tagged, the characteristics of the line
     are set appropriately.  A line that is added, is set to NEW; a line
     that is changed is set to CHANGE, and a line that is tagged with the
     <TAG> command, is set to TAG.  All three characteristics can be on
     at the one time.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     NOCHANGE NONEW NOTAG

SEE ALSO
     <TAG>, <SET HIGHLIGHT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Lineflag(CHARTYPE *params)
#else
short Lineflag(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define LF_PARAMS  4
   CHARTYPE *word[LF_PARAMS+1];
   CHARTYPE strip[LF_PARAMS];
   LINETYPE num_lines=0L,true_line=0L;
   CHARTYPE *save_params;
   short num_params=0,num_flags;
   short rc=RC_OK;
   short target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL;
   short save_target_type=TARGET_UNFOUND;
   TARGET target;
   bool num_lines_based_on_scope=FALSE,no_flag=FALSE;
   unsigned int new_flag=2;
   unsigned int changed_flag=2;
   unsigned int tag_flag=2;
   int i,j;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("comm4.c:   Lineflag");
   save_params = my_strdup( params );
   if ( save_params == NULL )
   {
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }

   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_NONE;
   num_params = param_split(params,word,LF_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 0)                                     /* no params */
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      (*the_free)( save_params );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   for (i = 0; i < num_params; i++)
   {
      if (!equal((CHARTYPE *)"change",word[i],3)
      &&  !equal((CHARTYPE *)"nochange",word[i],4)
      &&  !equal((CHARTYPE *)"new",word[i],3)
      &&  !equal((CHARTYPE *)"nonew",word[i],5)
      &&  !equal((CHARTYPE *)"tag",word[i],3)
      &&  !equal((CHARTYPE *)"notag",word[i],5))
      {
         no_flag = TRUE;
         break;
      }
   }
   /*
    * If we broke out of the above loop the first time, then
    * there are no valid flags specified
    */
   if ( i == 0 )
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      (*the_free)( save_params );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If no_flag is set to FALSE, all parameters are flags; therefore
    * the target will be 1
    */
   if ( no_flag == FALSE )
   {
      num_lines = 1L;
      true_line = get_true_line(TRUE);
      num_lines_based_on_scope = TRUE;
      num_flags = num_params;
   }
   else
   {
      if ( i+1 != num_params )
      {
         for ( j = 0; j < i; j++ )
         {
            strip[0]=STRIP_BOTH;
         }
         strip[i]=STRIP_NONE;
         num_params = param_split(params,word,i+1,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
      }
      initialise_target(&target);
      if  ( ( rc = validate_target( word[num_params-1], &target, target_type, get_true_line(TRUE), TRUE, TRUE ) ) != RC_OK )
      {
         free_target( &target );
         (*the_free)( save_params );
         TRACE_RETURN();
         return(rc);
      }
      switch ( target.rt[0].target_type )
      {
         case TARGET_BLOCK_CURRENT:
            switch( MARK_VIEW->mark_type )
            {
               case M_STREAM:
               case M_WORD:
               case M_COLUMN:
                  display_error(49,(CHARTYPE*)"",FALSE);
                  free_target(&target);
                  (*the_free)( save_params );
                  TRACE_RETURN();
                  return(RC_INVALID_OPERAND);
                  break;
               case M_BOX:
                  display_error(48,(CHARTYPE*)"",FALSE);
                  free_target(&target);
                  (*the_free)( save_params );
                  TRACE_RETURN();
                  return(RC_INVALID_OPERAND);
                  break;
               default:
                  break;
            }
            break;
         case TARGET_BLOCK_ANY:
            display_error(45,(CHARTYPE*)"",FALSE);
            free_target(&target);
            (*the_free)( save_params );
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
            break;
         default:
            break;
      }
      num_lines = target.num_lines;
      true_line = target.true_line;
      save_target_type = target.rt[0].target_type;
      num_lines_based_on_scope = (save_target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
      free_target(&target);
      num_flags = num_params-1;
   }
   for ( i = 0; i < num_flags; i++ )
   {
      if ( equal( (CHARTYPE *)"change", word[i], 3 ) )
         changed_flag = 1;
      else if ( equal( (CHARTYPE *)"nochange", word[i], 4 ) )
         changed_flag = 0;
      else if ( equal( (CHARTYPE *)"new", word[i], 3 ) )
         new_flag = 1;
      else if ( equal( (CHARTYPE *)"nonew", word[i], 5 ) )
         new_flag = 0;
      else if ( equal( (CHARTYPE *)"tag", word[i], 3 ) )
         tag_flag = 1;
      else if ( equal( (CHARTYPE *)"notag", word[i], 5 ) )
         tag_flag = 0;
      else
         ;
   }
   /*
    * Now we are here, everything's OK, do the actual modification...
    */
   rc = execute_set_lineflag( new_flag, changed_flag, tag_flag, true_line, num_lines, num_lines_based_on_scope, save_target_type );
   (*the_free)( save_params );
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set linend - allow/disallow multiple commands on command line

SYNTAX
     [SET] LINENd ON|OFF [character]

DESCRIPTION
     The SET LINEND command allows or disallows the execution of multiple
     commands on the <command line>. When setting LINEND ON, a 'character'
     is specified as the LINEND character which delimits each command.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     OFF #

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Linend(CHARTYPE *params)
#else
short Linend(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define LE_PARAMS  2
 CHARTYPE *word[LE_PARAMS+1];
 CHARTYPE strip[LE_PARAMS];
 unsigned short num_params=0;
 bool le_status=CURRENT_VIEW->linend_status;
 CHARTYPE le_value=CURRENT_VIEW->linend_value;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Linend");
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,LE_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 switch(num_params)
   {
    case 1:
    case 2:
           rc = execute_set_on_off(word[0],&le_status);
           if (rc != RC_OK)
             {
              display_error(1,word[0],FALSE);
              rc = RC_INVALID_OPERAND;
              break;
             }
           if (num_params == 1)
              break;
           if ((int)strlen((DEFCHAR *)word[1]) > (int)1)
             {
              display_error(1,word[1],FALSE);
              break;
             }
           le_value = word[1][0];
           break;
    case 0:
           display_error(3,(CHARTYPE *)"",FALSE);
           rc = RC_INVALID_OPERAND;
           break;
    default:
           display_error(2,(CHARTYPE *)"",FALSE);
           rc = RC_INVALID_OPERAND;
           break;
   }
 if (rc == RC_OK)
   {
    CURRENT_VIEW->linend_status = le_status;
    CURRENT_VIEW->linend_value = le_value;
   }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set macro - indicate if macros executed before commands

SYNTAX
     SET MACRO ON|OFF

DESCRIPTION
     The SET MACRO command allows the user to determine if macros
     are executed before a built-in command of the same name.

     This command MUST be prefixed with <SET> to distinguish it
     from the <MACRO> command.

     A macro with the same name as a built-in command will only
     be executed before the built-in command if <SET IMPMACRO>
     is ON, <SET MACRO> is ON, and the command was NOT executed
     with the <COMMAND> command.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: N/A

DEFAULT
     OFF

SEE ALSO
     <MACRO>, <SET IMPMACRO>, <COMMAND>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short SetMacro(CHARTYPE *params)
#else
short SetMacro(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:SetMacro");
 rc = execute_set_on_off(params,&CURRENT_VIEW->macro);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set macroext - set default macro extension value

SYNTAX
     [SET] MACROExt [ext]

DESCRIPTION
     The SET MACROEXT command sets the value of the file extension to be
     used for <macro> files. When a macro file name is specified on the
     <command line>, a period '.', then this value will be appended.
     If no value is specified for 'ext', then THE assumes that the
     supplied macro file name is the fully specified name for a macro.

     The length of 'ext' must be 10 characters or less.

     The macro extension is only appended to a file if that file does
     not include any path specifiers.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     the

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Macroext(CHARTYPE *params)
#else
short Macroext(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Macroext");
/*---------------------------------------------------------------------*/
/* If no value is specified for ext, set the value of macro_suffix to  */
/* "", otherwise set it to the supplied value, prefixed with '.'       */
/*---------------------------------------------------------------------*/
 if (strlen((DEFCHAR *)params) == 0)
    strcpy((DEFCHAR *)macro_suffix,"");
 else
   {
    if ((int)strlen((DEFCHAR *)params) > (int)10)
      {
       display_error(85,(CHARTYPE *)params,FALSE);
       TRACE_RETURN();
       return(RC_INVALID_OPERAND);
      }
    strcpy((DEFCHAR *)macro_suffix,".");
    strcat((DEFCHAR *)macro_suffix,(DEFCHAR *)params);
   }
 TRACE_RETURN();
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set macropath - set default path for macro commands

SYNTAX
     [SET] MACROPath PATH|path[s]

DESCRIPTION
     The SET MACROPATH command sets up the search path from which macro
     command files are executed. Each directory is seperated by a
     colon (Unix) or semi-colon (DOS & OS/2). Only 20 directories are
     allowed to be specified.

     When 'PATH' is specified, the search path is set to the system
     PATH environment variable.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Incompatible.

DEFAULT
     Path specified by env variable THE_MACRO_PATH

SEE ALSO
     <MACRO>, <SET IMPMACRO>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Macropath(CHARTYPE *params)
#else
short Macropath(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#if defined(UNIX)
# define PATH_DELIM ':'
#else
# define PATH_DELIM ';'
#endif
   register int len=0;
   int num_dirs,i;
   DEFCHAR *ptr=NULL;
   CHARTYPE *src;

   TRACE_FUNCTION( "commset1.c:Macropath" );
   /*
    * No checking is done on macro path supplied other than it contains a
    * value. Path delimiters are translated if necessary.
    */
   if ( strlen( (DEFCHAR *)params ) == 0 )
   {
      display_error( 3, (CHARTYPE *)"", FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if ( my_stricmp( (DEFCHAR *)params, "PATH" ) == 0 )
      src = (CHARTYPE *)getenv( "PATH" );
   else
      src = params;
   strcpy( (DEFCHAR *)the_macro_path_buf, (DEFCHAR *)src );
   strrmdup( strtrans( the_macro_path_buf, OSLASH, ISLASH ), ISLASH, TRUE );
   strrmdup( the_macro_path_buf, PATH_DELIM, FALSE );
   len = strlen( (DEFCHAR *)the_macro_path_buf );
   if ( the_macro_path_buf[len-1] == PATH_DELIM )
   {
      the_macro_path_buf[len-1] = '\0';
      len--;
   }
   if ( len == 0 )
   {
      display_error( 3, (CHARTYPE *)"", FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   strcpy( (DEFCHAR *)the_macro_path, (DEFCHAR *)the_macro_path_buf );
   /*
    * Count the number of PATH_DELIM in the buffer to determine
    * how many pointers to allocate.
    */
   for ( num_dirs = 1, i = 0; i < len; i++ )
   {
      if ( *(the_macro_path+i) == PATH_DELIM )
         num_dirs++;
   }
   /*
    * If we have already allocated enough pointer memory
    * don't bother allocating more
    */
   if ( num_dirs > total_macro_dirs )
   {
      if ( the_macro_dir == NULL )
         the_macro_dir = (CHARTYPE **)(*the_malloc)( num_dirs * sizeof(DEFCHAR *) );
      else
         the_macro_dir = (CHARTYPE **)(*the_realloc)( the_macro_dir, num_dirs * sizeof(DEFCHAR *) );
      if ( the_macro_dir == NULL )
      {
         max_macro_dirs = total_macro_dirs = 0;
         display_error( 30, (CHARTYPE *)"",FALSE );
         TRACE_RETURN();
         return(RC_OUT_OF_MEMORY);
      }
      total_macro_dirs = num_dirs;
   }
   the_macro_dir[0] = the_macro_path_buf;
   max_macro_dirs = 0;
   for ( ptr = (DEFCHAR*)the_macro_path_buf; *ptr != '\0'; ptr++ )
   {
      if ( *ptr == PATH_DELIM )
      {
         *ptr = '\0';
         the_macro_dir[++max_macro_dirs] = (CHARTYPE*)++ptr;
      }
   }
   max_macro_dirs++;
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set margins - set left and right margins for wordwrap

SYNTAX
     [SET] MARgins left right [[+|-]indent]

DESCRIPTION
     The SET MARGINS command sets the 'left' and 'right' margins and the
     number of columns to 'indent' a paragraph.

     These values are used with the <SET WORDWRAP> option.

     All options can be specified as the current EQUIVCHAR to retain the
     existing value.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     1 72 +0

SEE ALSO
     <SET WORDWRAP>, <SET EQUIVCHAR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Margins(CHARTYPE *params)
#else
short Margins(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define MAR_PARAMS  3
   CHARTYPE *word[MAR_PARAMS+1];
   CHARTYPE strip[MAR_PARAMS];
   short num_params=0;
   short left=0,right=0,indent=0;
   bool offset=FALSE,consistancy_error=FALSE;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commset1.c:Margins");
   /*
    * Two parameters are mandatory, the third is optional.
    */
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   num_params = param_split(params,word,MAR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 2)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (num_params > 3)
   {
      display_error(2,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Parse the parameters...
    */
   if ( equal(EQUIVCHARstr, word[0], 1 ) )
   {
      left = CURRENT_VIEW->margin_left;
   }
   else
   {
      left = atoi((DEFCHAR *)word[0]);
      if (left < 1)
      {
         display_error(5,word[0],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
   }
   /*
    * Right margin value can be *, set to maximum line length.
    */
   if ( equal( EQUIVCHARstr, word[1], 1 ) )
   {
      right = CURRENT_VIEW->margin_right;
   }
   else if ( equal( (CHARTYPE *)"*", word[1], 1 ) )
   {
      right = max_line_length;
   }
   else
   {
      right = atoi((DEFCHAR *)word[1]);
      if (right > max_line_length)
      {
         display_error(6,word[1],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      if (right < 1)
      {
         display_error(5,word[1],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
   }
   /*
    * Left margin must be less than right margin.
    */
   if (right < left)
   {
      display_error(5,word[1],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Obtain current values for indent, in case they aren't changed by
    * the current command. (ie. no third parameter)
    */
   indent = CURRENT_VIEW->margin_indent;
   offset = CURRENT_VIEW->margin_indent_offset_status;
   /*
    * Determine the type of offset for the indent value. If a sign is
    * specified, then the number supplied is relative to the left margin
    * otherwise it is an absolute column value.
    * Do the following processing only if the indent parameter is NOT
    * the EQUIVCHAR.
    */
   if ( num_params == 3
   &&   !equal( EQUIVCHARstr, word[2], 1 ) )
   {
      if (*(word[2]) == '-'
      ||  *(word[2]) == '+')
      {
         offset = TRUE;
         if ((indent = atoi((DEFCHAR *)word[2])) == 0)
         {
            if (strcmp((DEFCHAR *)word[2],"+0") != 0)
            {
               display_error(1,word[2],FALSE);
               TRACE_RETURN();
               return(RC_INVALID_OPERAND);
            }
         }
      }
      else
      {
         offset = FALSE;
         /*
          * Absolute indent cannot be negative.
          */
         if ((indent = atoi((DEFCHAR *)word[2])) < 0)
         {
            display_error(1,word[2],FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
         }
      }
   }
   /*
    * Once all values are determined, validate the relationship between
    * the margins and the indent values.
    * Rules:
    *       o If indent is a negative offset, the resultant column value
    *         cannot be negative.
    *       o If indent is a positive offset, the resultant column value
    *         cannot be > max_line_length or right margin
    *       o If indent is an absolute value, it cannot be > right margin
    */
   consistancy_error = FALSE;
   if (offset
   && indent < 0
   && indent + left < 0)
      consistancy_error = TRUE;
   if (offset
   && indent > 0
   && indent + left > right)
      consistancy_error = TRUE;
   if (offset
   && indent > 0
   && (LENGTHTYPE)(indent + left) > max_line_length)
      consistancy_error = TRUE;
   if (!offset
   && indent > right)
      consistancy_error = TRUE;
   if (consistancy_error)
     {
      if (offset)
         sprintf((DEFCHAR *)temp_cmd,"%d %d %+d",left,right,indent);
      else
         sprintf((DEFCHAR *)temp_cmd,"%d %d %d",left,right,indent);
      display_error(12,temp_cmd,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * All OK, so save the values...
    */
   CURRENT_VIEW->margin_left = left;
   CURRENT_VIEW->margin_right = right;
   CURRENT_VIEW->margin_indent = indent;
   CURRENT_VIEW->margin_indent_offset_status = offset;
   /*
    * If the SCALE line is currently displayed, display the page so that
    * any changes are reflected in the SCALE line. Also display page if
    * boundmark is not off.
    */
   if ( CURRENT_VIEW->scale_on
   ||   CURRENT_VIEW->boundmark != BOUNDMARK_OFF )
   {
      build_screen(current_screen);
      display_screen(current_screen);
   }
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set mouse - turn mouse support on or off

SYNTAX
     [SET] MOUSE ON|OFF

DESCRIPTION
     The SET MOUSE command allows the user to turn on or off mouse
     support in THE.  With mouse support, THE commands assigned to
     a mouse button event will be executed.  See APPENDIX 3 for
     details on default mouse support.

     If the platform does not support mouse operations, the default
     setting will be OFF.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible. Does not support all options.

DEFAULT
     ON - if mouse supported, OFF - otherwise

SEE ALSO
     <DEFINE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Mouse(CHARTYPE *params)
#else
short Mouse(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Mouse");
 rc = execute_set_on_off(params,&MOUSEx);
#if defined(PDCURSES_MOUSE_ENABLED)
 mouse_set((MOUSEx)?ALL_MOUSE_EVENTS:0L);
#endif
#if defined(NCURSES_MOUSE_VERSION)
 mousemask((MOUSEx)?ALL_MOUSE_EVENTS:0, (mmask_t*)NULL);
#endif
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set msgline - set position and size of message line

SYNTAX
     [SET] MSGLine ON M[+n|-n]|[+|-]n [lines] [Overlay]
     [SET] MSGLine CLEAR

DESCRIPTION
     The SET MSGLINE set command specifies the position of the
     <message line> and the size of the message line window.

     The first form of positional parameters is:

     M[+n|-n]
     this sets the first line to be relative to the middle of
     the screen. A positive value adds to the middle line number,
     a negative subtracts from it.
     eg. M+3 on a 24 line screen will be line 15
         M-5 on a 24 line screen will be line 7

     The second form of positional parameters is:

     [+|-]n
     this sets the first line to be relative to the top of the
     screen (if positive or no sign) or relative to the bottom
     of the screen if negative.
     eg. +3 or 3 will set first line to line 3
         -3 on a 24 line screen will set first line to line 21

     If the resulting line is outside the bounds of the screen
     the position of the message line will become the middle line
     on the screen.

     The 'lines' argument specifies the maximum number of lines of
     error messages to display at the one time.  If this value is
     specified as a whole number it must be less than or equal to the
     number of lines that could fit on the screen from the starting row.
     '*' can be specified to indicate that as many lines as possible should
     be displayed.

     All options can be specified as the current EQUIVCHAR to retain the
     existing value.

     The second format of the command clears the messages being displayed.
     This is useful in macros where you need to display an error message
     but also want to be able to clear it.

COMPATIBILITY
     XEDIT: Compatible.
            The OVERLAY option is the default but ignored.
            The second format is not supported.
     KEDIT: Compatible
            The OVERLAY option is the default but ignored.
            The second format is not supported.

DEFAULT
     ON 2 5 Overlay

SEE ALSO
     <SET EQUIVCHAR>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Msgline(CHARTYPE *params)
#else
short Msgline(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define MSG_PARAMS  5
   CHARTYPE *word[MSG_PARAMS+1];
   CHARTYPE strip[MSG_PARAMS];
   short num_params=0;
   short rc=RC_OK;
   short base=(short)CURRENT_VIEW->msgline_base;
   short off=CURRENT_VIEW->msgline_off;
   int start_row;
   bool msgsts=FALSE;
   int num_lines=CURRENT_VIEW->msgline_rows;

   TRACE_FUNCTION( "commset1.c:Msgline" );
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   strip[4]=STRIP_NONE;
   num_params = param_split( params, word, MSG_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE );
   /*
    * If only 1 parameter, it must be CLEAR...
    */
   if ( num_params == 1 )
   {
      if ( equal( (CHARTYPE *)"CLEAR", word[0], 5 ) )
      {
         clear_msgline( -1 );
         TRACE_RETURN();
         return(RC_OK);
      }
   }
   /*
    * more than 1 parameter or only parameter not CLEAR
    */
   if ( num_params < 2 )
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (num_params > 4)
   {
      display_error(2,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Parse the status parameter...
    */
   if ( equal( EQUIVCHARstr, word[0], 1 ) )
   {
      msgsts = TRUE;
   }
   else
   {
      rc = execute_set_on_off( word[0], &msgsts );
      if (rc != RC_OK)
      {
         TRACE_RETURN();
         return(rc);
      }
   }
   /*
    * ... only "ON" is allowed...
    */
   if (!msgsts)
   {
      display_error(1,word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Parse the position parameter...
    */
   if (num_params > 1)
   {
      if ( equal( EQUIVCHARstr, word[1], 1 ) )
      {
         base = CURRENT_VIEW->msgline_base;
         off = CURRENT_VIEW->msgline_off;
      }
      else
      {
         rc = execute_set_row_position( word[1], &base, &off );
         if (rc != RC_OK)
         {
            TRACE_RETURN();
            return(rc);
         }
      }
   }
   /*
    * To get here we have either two arguments or one. If two, the first
    * is the number of lines, and the second MUST be Overlay.
    * If one argument, it is either Overlay or number of lines.
    */
   switch(num_params)
   {
      case 3:
         if (equal((CHARTYPE *)"overlay",word[2],1))
            num_lines = 1;
         else if ( equal( (CHARTYPE *)EQUIVCHARstr, word[2], 1 ) )
            num_lines = CURRENT_VIEW->msgline_rows;
         else if ( equal( (CHARTYPE *)"*", word[2], 1 ) )
            num_lines = 0;
         else
         {
            num_lines = atoi( (DEFCHAR *)word[2] );
            if (num_lines < 1)
            {
               display_error( 5, word[2], FALSE );
               TRACE_RETURN();
               return(rc);
            }
            start_row = calculate_actual_row( base, off, CURRENT_SCREEN.screen_rows, TRUE );
            if ( base == POSITION_BOTTOM )
            {
               if ( num_lines > start_row )
                  rc = RC_INVALID_OPERAND;
            }
            else
            {
               if ( start_row + num_lines > CURRENT_SCREEN.screen_rows )
                  rc = RC_INVALID_OPERAND;
            }
            if ( rc == RC_INVALID_OPERAND )
            {
               display_error( 6, word[2], FALSE );
               TRACE_RETURN();
               return(rc);
            }
         }
         break;
      case 4:
         if ( equal( (CHARTYPE *)EQUIVCHARstr, word[2], 1 ) )
            num_lines = CURRENT_VIEW->msgline_rows;
         else if ( equal( (CHARTYPE *)"*", word[2], 1 ) )
            num_lines = 0;
         else
         {
            num_lines = atoi( (DEFCHAR *)word[2] );
            if (num_lines < 1)
            {
               display_error( 5, word[2], FALSE );
               TRACE_RETURN();
               return(rc);
            }
            start_row = calculate_actual_row( base, off, CURRENT_SCREEN.screen_rows, TRUE );
            if ( base == POSITION_BOTTOM )
            {
               if ( num_lines > start_row )
                  rc = RC_INVALID_OPERAND;
            }
            else
            {
               if ( start_row + num_lines > CURRENT_SCREEN.screen_rows )
                  rc = RC_INVALID_OPERAND;
            }
            if ( rc == RC_INVALID_OPERAND )
            {
               display_error( 6, word[2], FALSE );
               TRACE_RETURN();
               return(rc);
            }
         }
         if ( !equal((CHARTYPE *)"overlay", word[3], 1 )
         &&   !equal((CHARTYPE *)EQUIVCHARstr, word[3], 1 ) )
         {
            display_error( 1, word[3], FALSE );
            TRACE_RETURN();
            return(rc);
         }
         break;
      default:
         num_lines = 1;
         break;
   }
   CURRENT_VIEW->msgline_base = (CHARTYPE)base;
   CURRENT_VIEW->msgline_off = off;
   CURRENT_VIEW->msgline_rows = num_lines;
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set msgmode - set display of messages on or off

SYNTAX
     [SET] MSGMode ON|OFF

DESCRIPTION
     The SET MSGMODE set command determines whether error messages will
     be displayed or suppressed.

COMPATIBILITY
     XEDIT: Does not support [Short|Long] options.
     KEDIT: Compatible

DEFAULT
     ON

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Msgmode(CHARTYPE *params)
#else
short Msgmode(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Msgmode");
 rc = execute_set_on_off(params,&CURRENT_VIEW->msgmode_status);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set newlines - set position of cursor after adding blank line

SYNTAX
     [SET] NEWLines Aligned|Left

DESCRIPTION
     The SET NEWLINES set command determines where the cursor displays
     after a new line is added to the file.

     With 'ALIGNED', the cursor will display in the column of the new line
     immediately underneath the first non-blank character in the line
     above.
     With 'LEFT', the cursor will display in the first column of the new
     line.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Same command, different functionality.

DEFAULT
     Aligned

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Newlines(CHARTYPE *params)
#else
short Newlines(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define NEW_PARAMS  1
 CHARTYPE parm[NEW_PARAMS];
 CHARTYPE *word[NEW_PARAMS+1];
 CHARTYPE strip[NEW_PARAMS];
 unsigned short num_params=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Newlines");
 strip[0]=STRIP_BOTH;
 num_params = param_split(params,word,NEW_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params > 1)
   {
    display_error(2,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 if (num_params < 1)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }

 parm[0] = (CHARTYPE)UNDEFINED_OPERAND;
 if (equal((CHARTYPE *)"aligned",word[0],1))
    parm[0] = TRUE;
 if (equal((CHARTYPE *)"left",word[0],1))
    parm[0] = FALSE;
 if (parm[0] == (CHARTYPE)UNDEFINED_OPERAND)
   {
    display_error(1,word[0],FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 CURRENT_VIEW->newline_aligned = parm[0];
 TRACE_RETURN();
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set nondisp - specify character to display for non-displaying characters

SYNTAX
     [SET] NONDisp character

DESCRIPTION
     The SET NONDISP command allows the user to change the 'character'
     that is displayed for non-displaying commands when <SET ETMODE>
     is OFF.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: N/A

DEFAULT
     #

SEE ALSO
     <SET ETMODE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Nondisp(CHARTYPE *params)
#else
short Nondisp(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Nondisp");
 if (strlen((DEFCHAR *)params) != 1)
   {
    display_error(1,params,FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 NONDISPx = *params;
 build_screen(current_screen);
 display_screen(current_screen);
 TRACE_RETURN();
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set number - turn prefix numbers on or off

SYNTAX
     [SET] NUMber ON|OFF

DESCRIPTION
     The SET NUMBER command allows the user to toggle the display of
     numbers in the <prefix area>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     ON

SEE ALSO
     <SET PREFIX>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Number(CHARTYPE *params)
#else
short Number(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commset1.c:Number");
 rc = execute_set_on_off(params,&CURRENT_VIEW->number);
 if (rc == RC_OK)
   {
    build_screen(current_screen);
    display_screen(current_screen);
   }
 TRACE_RETURN();
 return(rc);
}
