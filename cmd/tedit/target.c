/***********************************************************************/
/* TARGET.C - Functions related to targets.                            */
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

static char RCSid[] = "$Id: target.c,v 1.12 2002/06/07 22:25:05 mark Exp $";

#include <the.h>
#include <proto.h>

#define STATE_START    0
#define STATE_DELIM    1
#define STATE_STRING   2
#define STATE_BOOLEAN  3
#define STATE_NEXT     4
#define STATE_POINT    5
#define STATE_ABSOLUTE 6
#define STATE_RELATIVE 7
#define STATE_POSITIVE 8
#define STATE_NEGATIVE 9
#define STATE_REGEXP_START   10
#define STATE_REGEXP   11
#define STATE_SPARE    12
#define STATE_QUIT     98
#define STATE_ERROR    99

#ifdef HAVE_PROTO
static bool is_blank(LINE *);
#else
static bool is_blank();
#endif

/***********************************************************************/
#ifdef HAVE_PROTO
short split_change_params(CHARTYPE *cmd_line,CHARTYPE **old_str,CHARTYPE **new_str,
                          TARGET *target,LINETYPE *num,LINETYPE *occ)
#else
short split_change_params(cmd_line,old_str,new_str,target,num,occ)
CHARTYPE *cmd_line,**old_str,**new_str;
TARGET *target;
LINETYPE *num,*occ;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define SCP_PARAMS  2
 CHARTYPE *word[SCP_PARAMS+1];
 CHARTYPE strip[SCP_PARAMS];
 register short i=0,j=0;
 CHARTYPE *target_start=NULL;
 short rc=RC_OK;
 CHARTYPE delim=' ';
 short idx=0;
 short target_type = TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL|TARGET_SPARE;
 unsigned short num_params=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("target.c:  split_change_params");
/*---------------------------------------------------------------------*/
/* First, determine the delimiter; the first character in the argument */
/* string.                                                             */
/*---------------------------------------------------------------------*/
 delim = *(cmd_line);
/*---------------------------------------------------------------------*/
/* Set up default values for the return values...                      */
/*---------------------------------------------------------------------*/
 *old_str = cmd_line+1;
 *new_str = (CHARTYPE *)"";
 target_start = (CHARTYPE *)"";
 *num = *occ = 1L;
 target->num_lines = 1L;
 target->true_line = get_true_line(TRUE);
/*---------------------------------------------------------------------*/
/* Set up default values for the return values...                      */
/*---------------------------------------------------------------------*/
 idx = strlen((DEFCHAR *)cmd_line);
 for (i=1,j=0;i<idx;i++)
   {
    if (*(cmd_line+i) == delim)
      {
       j++;
       switch(j)
         {
          case 1:
               *(cmd_line+i) = '\0';
               *new_str = cmd_line+i+1;
               break;
          case 2:
               *(cmd_line+i) = '\0';
               target_start = cmd_line+i+1;
               break;
          default:
               break;
         }
      }
    if (j == 2)
       break;
   }
/*---------------------------------------------------------------------*/
/* Check to see if there is a target, if not return here.              */
/*---------------------------------------------------------------------*/
 if (blank_field(target_start))
   {
    TRACE_RETURN();
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* Parse and validate the target...                                    */
/*---------------------------------------------------------------------*/
 if ((rc = validate_target(target_start,target,target_type,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
   {
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* Check to see if there are further arguments after the target...     */
/*---------------------------------------------------------------------*/
 if (target->spare == (-1))
   {
    TRACE_RETURN();
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* Validate the arguments following the target...                      */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(strtrunc(target->rt[target->spare].string),word,SCP_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params == 1
 ||  num_params == 2)
   {
    if (strcmp((DEFCHAR *)word[0],"*") == 0)
       *num = max_line_length;
/*       *num = MAX_LONG;*/
    else
       if (!valid_positive_integer(word[0]))
         {
          display_error(4,word[0],FALSE);
          TRACE_RETURN();
          return(RC_INVALID_OPERAND);
         }
       else
          *num = min(atol((DEFCHAR *)word[0]),max_line_length);
   }
 if (num_params == 2)
   {
    if (!valid_positive_integer(word[1]))
      {
       display_error(4,word[1],FALSE);
       TRACE_RETURN();
       return(RC_INVALID_OPERAND);
      }
    else
       *occ = atol((DEFCHAR *)word[1]);
   }

 TRACE_RETURN();
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short parse_target(CHARTYPE *target_spec,LINETYPE true_line,TARGET *target,
                   short target_types,bool display_parse_error,
                   bool allow_error_display,bool column_target)
#else
short parse_target(target_spec,true_line,target,target_types,
                   display_parse_error,allow_error_display,column_target)
CHARTYPE *target_spec;
LINETYPE true_line;
TARGET *target;
short target_types;
bool display_parse_error,allow_error_display,column_target;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short num_targets=0;
   CHARTYPE boolean=' ';
   short state=STATE_NEXT;
   short len=0,inc=0,target_length=strlen((DEFCHAR *)target_spec),off=0;
   CHARTYPE delim=' ';
   register short i=0;
   register short j=0;
   register short k=0;
   short str_start=0,str_end=0;
   short rc=RC_OK;
   short potential_spare_start=0;
   bool negative=FALSE;
   CHARTYPE *ptr=NULL;
   LINETYPE lineno=0L;
   char regexp[7]="REGEXP";
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("target.c:  parse_target");
   /*
    * Copy the incoming target specification...
    */
   if ((target->string = (CHARTYPE *)my_strdup(target_spec)) == NULL)
   {
      if (allow_error_display)
         display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   ptr = target->string;
   /*
    * Parse the target...
    */
   switch(target_types)
   {
      case TARGET_FIND:
      case TARGET_NFIND:
      case TARGET_FINDUP:
      case TARGET_NFINDUP:
         for (i=0;i<target_length;i++)
         {
            if (*(ptr+i) == ' ')
               *(ptr+i) = CURRENT_VIEW->arbchar_single;
            else
               if (*(ptr+i) == '_')
                  *(ptr+i) = ' ';
         }
         target->rt = (RTARGET *)(*the_malloc)(sizeof(RTARGET));
         if (target->rt == NULL)
         {
            if (allow_error_display)
               display_error(30,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_OUT_OF_MEMORY);
         }
         target->num_targets = 1;
         target->rt[0].not = (target_types == TARGET_NFIND || target_types == TARGET_NFINDUP)?TRUE:FALSE;
         target->rt[0].negative = (target_types == TARGET_FINDUP || target_types == TARGET_NFINDUP)?TRUE:FALSE;
         target->rt[0].boolean = ' ';
         target->rt[0].found = FALSE;
         target->rt[0].length = target_length;
         target->rt[0].target_type = TARGET_STRING;
         target->rt[0].numeric_target = 0L;
         target->rt[0].have_compiled_re = FALSE;
         target->rt[0].string = (CHARTYPE *)(*the_malloc)((target_length*sizeof(CHARTYPE))+1);
         if (target->rt[0].string == (CHARTYPE *)NULL)
         {
            if (allow_error_display)
               display_error(30,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_OUT_OF_MEMORY);
         }
         strcpy((DEFCHAR *)target->rt[0].string,(DEFCHAR *)ptr);
         TRACE_RETURN();
         return(RC_OK);
         break;
      default:
         break;
   }
   while(1)
   {
      inc = 1;
      switch(state)
      {
         case STATE_NEXT:
            if (target->rt == NULL)
               target->rt = (RTARGET *)(*the_malloc)((num_targets+1)*sizeof(RTARGET));
            else
               target->rt = (RTARGET *)(*the_realloc)(target->rt,(num_targets+1)*sizeof(RTARGET));
            if (target->rt == NULL)
            {
               if (allow_error_display)
                  display_error(30,(CHARTYPE *)"",FALSE);
               TRACE_RETURN();
               return(RC_OUT_OF_MEMORY);
            }
            target->rt[num_targets].not = FALSE;
            target->rt[num_targets].boolean = boolean;
            target->rt[num_targets].found = FALSE;
            target->rt[num_targets].length = 0;
            target->rt[num_targets].string = NULL;
            target->rt[num_targets].negative = FALSE;
            target->rt[num_targets].target_type = TARGET_ERR;
            target->rt[num_targets].numeric_target = 0L;
            target->rt[num_targets].have_compiled_re = FALSE;
            if (target->spare != (-1))
               state = STATE_SPARE;
            else
               state = STATE_START;
            inc = 0;
            break;
         case STATE_START:
            switch(*(ptr+i))
            {
               case '~': case '^':
                  if (target->rt[num_targets].not)
                  {
                     state = STATE_ERROR;
                     inc = 0;
                     break;
                  }
                  target->rt[num_targets].not = TRUE;
                  break;
               case ' ': 
               case '\t':
                  break;
               case '-':
                  if (target->rt[num_targets].negative)
                  {
                     state = STATE_ERROR;
                     inc = 0;
                     break;
                  }
                  target->rt[num_targets].negative = TRUE;
                  state = STATE_NEGATIVE;
                  break;
               case 'r': case 'R':
                  if (target->rt[num_targets].not
                  ||  target->rt[num_targets].negative)
                  {
                     state = STATE_ERROR;
                     inc = 0;
                     break;
                  }
                  for ( k = 0; k < 6; k++ )
                  {
                     if ( i+k > target_length )
                     {
                        state = STATE_ERROR;
                        break;
                     }
                     if ( toupper(*(ptr+i+k)) == regexp[k] )
                        continue;
                     if ( *(ptr+(i+k)) == ' '
                     ||   *(ptr+(i+k)) == '\0'
                     ||   *(ptr+(i+k)) == '\t' )
                     {
                        state = STATE_REGEXP_START;
                        break;
                     }
                  }
                  switch( state )
                  {
                     case STATE_START:
                        if ( *(ptr+(i+k)) == ' '
                        ||   *(ptr+(i+k)) == '\0'
                        ||   *(ptr+(i+k)) == '\t' )
                        {
                           state = STATE_REGEXP_START;
                           inc = k;
                           break;
                        }
                        else
                        {
                           state = STATE_ERROR;
                           inc = 0;
                           break;
                        }
                        break;
                     case STATE_REGEXP_START:
                        inc = k;
                        break;
                     default:
                        state = STATE_ERROR;
                        inc = 0;
                        break;
                  }
                  potential_spare_start = i+k+1;
                  break;
               case '+':
                  if (target->rt[num_targets].negative)
                  {
                     state = STATE_ERROR;
                     inc = 0;
                     break;
                  }
/*                    inc = 1; */
                  target->rt[num_targets].negative = FALSE;
                  state = STATE_POSITIVE;
                  break;
               case '.':
                  if (target->rt[num_targets].negative)
                  {
                     state = STATE_ERROR;
                     inc = 0;
                     break;
                  }
                  state = STATE_POINT;
                  str_start = i;
                  break;
               case '*':
                  state = STATE_BOOLEAN;
                  target->rt[num_targets].target_type = TARGET_RELATIVE;
                  target->rt[num_targets].string = (CHARTYPE *)(*the_malloc)(3);
                  if (target->rt[num_targets].string == NULL)
                  {
                     if (allow_error_display)
                        display_error(30,(CHARTYPE *)"",FALSE);
                     TRACE_RETURN();
                     return(RC_OUT_OF_MEMORY);
                  }
                  if (target->rt[num_targets].negative)
                  {
                     if (column_target)
                        target->rt[num_targets].numeric_target = (LINETYPE)CURRENT_VIEW->zone_start - true_line - 1L;
                     else
                        target->rt[num_targets].numeric_target = true_line*(-1L);
                     strcpy((DEFCHAR *)target->rt[num_targets].string,"-*");
                  }
                  else
                  {
                     if (column_target)
                        target->rt[num_targets].numeric_target = (LINETYPE)((LINETYPE)CURRENT_VIEW->zone_end - true_line) + 1L;
                     else
                        target->rt[num_targets].numeric_target = (CURRENT_FILE->number_lines - true_line) + 2L;
                     strcpy((DEFCHAR *)target->rt[num_targets].string,"*");
                  }
                  inc = 1;
                  potential_spare_start = i+1;
                  num_targets++;
                  break;
               case ':': case ';':
                  state = STATE_ABSOLUTE;
                  delim = *(ptr+i);
                  str_start = i+1;
                  break;
               case '/': case '\\': case '@': case '`': case '#': case '$':
               case '%': case '(': case ')': case '{': case '}': case '[': case ']':
               case '"': case '\'': case '<': case '>': 
               case ',':
                  state = STATE_STRING;
                  str_start = i+1;
                  delim = *(ptr+i);
                  break;
               case 'a': case 'A':
                  if (target->rt[num_targets].not
                  ||  target->rt[num_targets].negative)
                  {
                     state = STATE_ERROR;
                     inc = 0;
                     break;
                  }
                  if (target_length-i < 3)
                  {
                     target->rt[num_targets].target_type = TARGET_ERR;
                     state = STATE_ERROR;
                     inc = 0;
                     break;
                  }
                  if (memcmpi((CHARTYPE *)"all",ptr+i,3) == 0
                  && (*(ptr+(i+3)) == ' ' 
                      || *(ptr+(i+3)) == '\0'
                      || *(ptr+(i+3)) == '\t'))
                  {
                     target->rt[num_targets].target_type = TARGET_ALL;
                     inc = 3;
                     state = STATE_BOOLEAN;
                     num_targets++;
                     potential_spare_start = i+3;
                     break;
                  }
                  state = STATE_ERROR;
                  inc = 0;
                  break;
               case 'b': case 'B':
                  if (target_length-i < 5)
                  {
                     state = STATE_ERROR;
                     inc = 0;
                     break;
                  }
                  if (memcmpi((CHARTYPE *)"blank",ptr+i,5) == 0
                  && (*(ptr+(i+5)) == ' ' 
                      || *(ptr+(i+5)) == '\0'
                      || *(ptr+(i+5)) == '\t'))
                  {
                     target->rt[num_targets].target_type = TARGET_BLANK;
                     inc = 5;
                     potential_spare_start = i+5;
                     state = STATE_BOOLEAN;
                     num_targets++;
                     break;
                  }
                  if (target->rt[num_targets].not
                  ||  target->rt[num_targets].negative)
                  {
                     state = STATE_ERROR;
                     inc = 0;
                     break;
                  }
                  if (memcmpi((CHARTYPE *)"block",ptr+i,5) == 0
                  && (*(ptr+(i+5)) == ' ' 
                      || *(ptr+(i+5)) == '\0'
                      || *(ptr+(i+5)) == '\t'))
                  {
                     target->rt[num_targets].target_type = TARGET_BLOCK;
                     inc = 5;
                     potential_spare_start = i+5;
                     state = STATE_BOOLEAN;
                     num_targets++;
                     break;
                  }
                  state = STATE_ERROR;
                  inc = 0;
                  break;
               case '0': case '1': case '2': case '3': case '4': 
               case '5': case '6': case '7': case '8': case '9':      
                  if (UNTAAx)
                     state = STATE_ABSOLUTE;
                  else
                     state = STATE_RELATIVE;
                  str_start = i;
                  delim = '\0';
                  inc = 0;
                  break;
               default:
                  state = STATE_ERROR;
                  inc = 0;
                  break;
            }
            break;
         case STATE_REGEXP_START:
            switch(*(ptr+i))
            {
               case ' ': 
               case '\t':
                  break;
               case '/': case '\\': case '@': case '`': case '#': case '$':
               case '%': case '(': case ')': case '{': case '}': case '[': case ']':
               case '"': case '\'': case '<': case '>': 
               case ',':
                  state = STATE_REGEXP;
                  str_start = i+1;
                  delim = *(ptr+i);
                  break;
               default:
                  state = STATE_ERROR;
                  inc = 0;
                  break;
            }
            break;
         case STATE_REGEXP:
            switch(*(ptr+i))
            {
               case '/': case '\\': case '@': case '`': case '#': case '$':
               case '%': case '(': case ')': case '{': case '}': case '[': case ']':
               case '"': case '\'': case '<': case '>': 
               case '\0':
                  if (*(ptr+i) == delim
                  ||  *(ptr+i) == '\0')
                  {
                     state = STATE_BOOLEAN;
                     str_end = i;
                     len = str_end-str_start;
                     target->rt[num_targets].string = (CHARTYPE *)(*the_malloc)(len+1);
                     if (target->rt[num_targets].string == NULL)
                     {
                        if (allow_error_display)
                           display_error(30,(CHARTYPE *)"",FALSE);
                        TRACE_RETURN();
                        return(RC_OUT_OF_MEMORY);
                     }
                     memcpy(target->rt[num_targets].string,ptr+str_start,len);
                     target->rt[num_targets].string[len] = '\0';
                     target->rt[num_targets].target_type = TARGET_REGEXP;
                     potential_spare_start = i+1;
                     num_targets++;
                  }
                  break;
               default:
                  break;
            }
            break;
         case STATE_STRING:
            switch(*(ptr+i))
            {
               case '/': case '\\': case '@': case '`': case '#': case '$':
               case '%': case '(': case ')': case '{': case '}': case '[': case ']':
               case '"': case '\'': case '<': case '>': 
               case '\0':
                  if (*(ptr+i) == delim
                  ||  *(ptr+i) == '\0')
                  {
                     state = STATE_BOOLEAN;
                     str_end = i;
                     len = str_end-str_start;
                     target->rt[num_targets].string = (CHARTYPE *)(*the_malloc)(len+1);
                     if (target->rt[num_targets].string == NULL)
                     {
                        if (allow_error_display)
                           display_error(30,(CHARTYPE *)"",FALSE);
                        TRACE_RETURN();
                        return(RC_OUT_OF_MEMORY);
                     }
                     memcpy(target->rt[num_targets].string,ptr+str_start,len);
                     target->rt[num_targets].string[len] = '\0';
                     target->rt[num_targets].length = len;
                     target->rt[num_targets].target_type = TARGET_STRING;
                     potential_spare_start = i+1;
                     num_targets++;
                  }
                  break;
               default:
                  break;
            }
            break;
         case STATE_BOOLEAN:
            switch(*(ptr+i))
            {
               case '\0':
                  break;
               case ' ': 
               case '\t':
                  break;
               case '&': 
               case '|':
                  state = STATE_NEXT;
                  boolean = *(ptr+i);
                  break;
               default:
                  if (target_types & TARGET_SPARE)
                  {
/*                       str_start = i;*/
                     str_start = potential_spare_start;
                     state = STATE_NEXT;
                     target->spare = 0; /* just to ensure state is set */
                     break;
                  }
                  state = STATE_ERROR;
                  inc = 0;
                  break;
            }
            break;
         case STATE_SPARE:
            switch(*(ptr+i))
            {
               case '\0':
                  str_end = i;
                  len = str_end-str_start;
                  target->rt[num_targets].string = (CHARTYPE *)(*the_malloc)(len+1);
                  if (target->rt[num_targets].string == NULL)
                  {
                     if (allow_error_display)
                        display_error(30,(CHARTYPE *)"",FALSE);
                     TRACE_RETURN();
                     return(RC_OUT_OF_MEMORY);
                  }
                  memcpy(target->rt[num_targets].string,ptr+str_start,len);
                  target->rt[num_targets].string[len] = '\0';
                  target->rt[num_targets].length = len;
                  target->rt[num_targets].target_type = TARGET_SPARE;
                  target->spare = num_targets;
                  num_targets++;
                  *(ptr+str_start) = '\0'; /* so target string does not include spare */
                  break;
               default:
                  break;
            }
            break;
         case STATE_ABSOLUTE:
         case STATE_RELATIVE:
            if (target->rt[num_targets].not)
            {
               state = STATE_ERROR;
               inc = 0;
               break;
            }
            switch(*(ptr+i))
            {
               case '\0':
               case ' ':
               case '\t':
                  str_end = i;
                  len = str_end-str_start;
                  target->rt[num_targets].string = (CHARTYPE *)(*the_malloc)(len+2);
                  if (target->rt[num_targets].string == NULL)
                  {
                     if (allow_error_display)
                        display_error(30,(CHARTYPE *)"",FALSE);
                     TRACE_RETURN();
                     return(RC_OUT_OF_MEMORY);
                  }
                  if (delim != '\0')
                  {
                     target->rt[num_targets].string[0] = delim;
                     off = 1;
                  }
                  else
                     off = 0;
                  memcpy(target->rt[num_targets].string+off,ptr+str_start,len);
                  target->rt[num_targets].string[len+off] = '\0';
                  target->rt[num_targets].length = len+off;
                  target->rt[num_targets].target_type = (state == STATE_ABSOLUTE) ? TARGET_ABSOLUTE : TARGET_RELATIVE;
                  target->rt[num_targets].numeric_target = atol((DEFCHAR *)target->rt[num_targets].string+off);
                  if (target->rt[num_targets].negative)
                     target->rt[num_targets].numeric_target *= (-1L);
                  if (state == STATE_ABSOLUTE)
                  {
                     if (column_target)
                     {
/*                          if (target->rt[num_targets].numeric_target < CURRENT_VIEW->current_column)*/
                        if (target->rt[num_targets].numeric_target < true_line)
                        {
                           target->rt[num_targets].negative = TRUE;
                           target->rt[num_targets].numeric_target = max(target->rt[num_targets].numeric_target,max(1,CURRENT_VIEW->zone_start-1));
                        }
                        else
                           target->rt[num_targets].numeric_target = min(target->rt[num_targets].numeric_target,min(max_line_length+1,CURRENT_VIEW->zone_end+1));
                     }
                     else
                     {
                        if (target->rt[num_targets].numeric_target < true_line)
                           target->rt[num_targets].negative = TRUE;
                        else
                           target->rt[num_targets].numeric_target = min(target->rt[num_targets].numeric_target,(CURRENT_FILE->number_lines+1L));
                     }
                  }
                  else
                  {
                     if (column_target)
                     {
                        if (target->rt[num_targets].negative)
                           target->rt[num_targets].numeric_target = max(target->rt[num_targets].numeric_target,((LINETYPE)CURRENT_VIEW->zone_start - true_line - 1L));
                        else
                           target->rt[num_targets].numeric_target = min(target->rt[num_targets].numeric_target,((LINETYPE)CURRENT_VIEW->zone_end - true_line + 1L));
                     }
                     else
                     {
                        if (target->rt[num_targets].negative)
                           target->rt[num_targets].numeric_target = max((target->rt[num_targets].numeric_target),(true_line == 0L) ? (0L) : (true_line*(-1L)));
                        else
                           target->rt[num_targets].numeric_target = min(target->rt[num_targets].numeric_target,(CURRENT_FILE->number_lines - true_line+1L));
                     }
                  }
                  potential_spare_start = i;
                  num_targets++;
                  state = STATE_BOOLEAN;
                  break;
               case '0': case '1': case '2': case '3': case '4':
               case '5': case '6': case '7': case '8': case '9':
                  break;
               default:
                  state = STATE_ERROR;
                  inc = 0;
                  break;
            }
            break;
         case STATE_NEGATIVE:
         case STATE_POSITIVE:
            switch(*(ptr+i))
            {
               case '0': case '1': case '2': case '3': case '4':
               case '5': case '6': case '7': case '8': case '9':
                  state = STATE_RELATIVE;
                  delim = (state==STATE_NEGATIVE) ? '-' : '+';
                  str_start = i;
                  inc = 0;
                  break;
               case '/': case '\\': case '@': case '`': case '#': case '$':
               case '%': case '(': case ')': case '{': case '}': case '[': case ']':
               case '"': case '\'': case '<': case '>': 
                  state = STATE_START;
                  inc = 0;
                  break;
               case '*':
                  state = STATE_START;
                  inc = 0;
                  break;
               case 'b': case 'B':
                  state = STATE_START;
                  inc = 0;
                  break;
               default:
                  state = STATE_ERROR;
                  inc = 0;
                  break;
            }
            break;
         case STATE_POINT:
            switch(*(ptr+i))
            {
               case ' ':
               case '\t':
                  state = STATE_BOOLEAN;
                  /* fall through */
               case '&':
               case '|':
               case '\0':
                  target->rt[num_targets].target_type = TARGET_POINT;
                  str_end = i;
                  len = str_end-str_start;
                  target->rt[num_targets].string = (CHARTYPE *)(*the_malloc)(len+1);
                  if (target->rt[num_targets].string == NULL)
                  {
                     if (allow_error_display)
                        display_error(30,(CHARTYPE *)"",FALSE);
                     TRACE_RETURN();
                     return(RC_OUT_OF_MEMORY);
                  }
                  memcpy(target->rt[num_targets].string,ptr+str_start,len);
                  target->rt[num_targets].string[len] = '\0';
                  target->rt[num_targets].length = len;
                  if (find_named_line(target->rt[num_targets].string,&lineno,TRUE) == NULL)
                  {
                     if (allow_error_display)
                        display_error(17,(CHARTYPE *)target->rt[num_targets].string,FALSE);
                     TRACE_RETURN();
                     return(RC_TARGET_NOT_FOUND);
                  }
                  target->rt[num_targets].numeric_target = lineno;
                  if (target->rt[num_targets].numeric_target < true_line)
                     target->rt[num_targets].negative = TRUE;
                  else
                     target->rt[num_targets].numeric_target = min(target->rt[num_targets].numeric_target,(CURRENT_FILE->number_lines+1L));
                  num_targets++;
                  potential_spare_start = i;
                  if (*(ptr+i) == ' ' || *(ptr+i) == '\t')
                     break;
                  boolean = *(ptr+i);
                  state = STATE_NEXT;
                  break;
               default:
                  break;
            }
            break;
         case STATE_ERROR:
            for (j=0;j<num_targets;j++)
               target->rt[j].target_type = TARGET_ERR;
            state = STATE_QUIT;
            break;
      }
      if (state == STATE_QUIT)
         break;
      i += inc;
      if (i > target_length) /* this allows for testing '\0' as delimiter */
         break;
   }
   target->num_targets = num_targets;
   if (num_targets == 0
   ||  target->rt[0].target_type == TARGET_ERR)
   {
      if (display_parse_error
      && allow_error_display)
         display_error(1,ptr,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Time to validate the targets we have parsed...
    *---------------------------------------------------------------------
    * Valid combinations are:
    * TARGET_ALL       (1 target only)
    *                  ALL  only
    * TARGET_BLOCK     (1 target only)
    *                 BLOCK  only
    *                  this section sets target_type to TARGET_BLOCK_ANY
    *                  or TARGET_BLOCK_CURRENT
    * TARGET_BLANK     (BLANK can be upper or lower case)
    *                  BLANK
    *                  -BLANK
    *                  ~BLANK
    *                  ~-BLANK
    * TARGET_STRING    (various valid delimiters)
    *                  /string/
    *                  -/string/
    *                  ~/string/
    *                  ~-/string/
    * TARGET_POINT
    *                  .xxxxxx
    *                  ~.xxxxxx
    * TARGET_ABSOLUTE
    *                  :99
    *                  ;99
    * TARGET_RELATIVE
    *                  99
    *                  +99
    *                  -99
    *                  *
    *                  +*
    *                  -*
    * TARGET_REGEXP    (various valid delimiters)
    *                  /regexp/
    *
    * Any of the above target types may be or'd together.
    *---------------------------------------------------------------------
    * For each of the targets, check its validity...
    *---------------------------------------------------------------------
    */
   negative = target->rt[0].negative;
   for (i=0;i<num_targets-((target->spare == (-1)) ? 0 : 1);i++)
   {
      switch(target->rt[i].target_type)
      {
         case TARGET_BLOCK:
            if (num_targets-((target->spare == (-1)) ? 0 : 1) != 1)
            {
               rc = RC_INVALID_OPERAND;
               break;
            }
            if (target_types & TARGET_BLOCK_ANY)
               target->rt[i].target_type = TARGET_BLOCK_ANY;
            else
            {
               if (target_types & TARGET_BLOCK_CURRENT)
                  target->rt[i].target_type = TARGET_BLOCK_CURRENT;
               else
                  rc = RC_INVALID_OPERAND;
            }
            break;
         case TARGET_ALL:
            if (num_targets-((target->spare == (-1)) ? 0 : 1) != 1)
            {
               rc = RC_INVALID_OPERAND;
               break;
            }
            if (target_types & target->rt[i].target_type)
               break;
            rc = RC_INVALID_OPERAND;
            break;
         case TARGET_REGEXP:
            if (num_targets-((target->spare == (-1)) ? 0 : 1) != 1)
            {
               rc = RC_INVALID_OPERAND;
               break;
            }
            if ( !(target_types & target->rt[i].target_type) )
            {
               rc = RC_INVALID_OPERAND;
               break;
            }
            /*
             * Compile the RE
             */
            memset( &target->rt[i].pattern_buffer, 0, sizeof(struct re_pattern_buffer) );
            ptr = (CHARTYPE *)re_compile_pattern( (DEFCHAR *)target->rt[i].string, REGEXPx, strlen( (DEFCHAR *)target->rt[i].string ), &target->rt[i].pattern_buffer );
            if (ptr)
            {
               /*
                * If ptr returns something, it is an error string
                * Display it if we are allowed to...
                */
               if ( display_parse_error
               &&   allow_error_display )
               {
                  sprintf( (DEFCHAR *)trec, "%s in %s", ptr, target->rt[i].string );
                  display_error( 216, (CHARTYPE *)trec, FALSE );
                  TRACE_RETURN();
                  return RC_INVALID_OPERAND;
               }
            }
            target->rt[i].have_compiled_re = TRUE;
            break;
         default:
            if (target->rt[i].negative != negative)
            {
               rc = RC_INVALID_OPERAND;
               break;
            }
            if (target_types & target->rt[i].target_type)
               break;
            rc = RC_INVALID_OPERAND;
            break;
      }
      if (rc == RC_INVALID_OPERAND)
         break;
   }
   /*
    * Display an error if anything found amiss and we are directed to
    * display an error...
    */
   if (rc != RC_OK
   && display_parse_error
   && allow_error_display)
      display_error(1,ptr,FALSE);
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
void initialise_target(TARGET *target)
#else
void initialise_target(target)
TARGET *target;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("target.c:  initialise_target");
   memset( target, 0, sizeof(TARGET) );
#if 0
   target->rt=NULL;
   target->string=NULL;
   target->num_lines = target->true_line = target->last_line = 0L;
   target->num_targets = 0;
   target->ignore_scope = FALSE;
#endif
   target->spare = (-1);
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void free_target(TARGET *target)
#else
void free_target(target)
TARGET *target;
#endif
/***********************************************************************/
{
   register short i=0;

   TRACE_FUNCTION("target.c:  free_target");
   if (target->string == NULL
   &&  target->num_targets == 0
   &&  target->rt == NULL)
   {
      TRACE_RETURN();
      return;
   }
   for (i=0;i<target->num_targets;i++)
   {
      if (target->rt[i].string != NULL)
         (*the_free)(target->rt[i].string);
      if (target->rt[i].have_compiled_re)
         the_regfree(&target->rt[i].pattern_buffer);
   }
   if (target->string != NULL)
      (*the_free)(target->string);
   if (target->rt != NULL)
      (*the_free)(target->rt);
   target->string = NULL;
   target->num_targets = 0;
   target->rt = NULL;
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short find_target(TARGET *target,LINETYPE true_line,bool display_parse_error,bool allow_error_display)
#else
short find_target(target,true_line,display_parse_error,allow_error_display)
TARGET *target;
LINETYPE true_line;
bool display_parse_error,allow_error_display;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   LINE *curr=NULL;
   LINETYPE num_lines=0L;
   LINETYPE line_number=0L;
   short status=RC_OK;

   TRACE_FUNCTION("target.c:  find_target");
   /*
    * Check single targets first (ALL and BLOCK)
    *---------------------------------------------------------------------
    * Check if first, and only target, is BLOCK...
    */
   switch(target->rt[0].target_type)
   {
      case TARGET_ALL:
         target->true_line = 1L;
         target->last_line = CURRENT_FILE->number_lines;
         target->num_lines = CURRENT_FILE->number_lines;
         TRACE_RETURN();
         return(RC_OK);
         break;
      case TARGET_BLOCK_ANY:
         if (MARK_VIEW == NULL)
         {
            if (allow_error_display)
               display_error(44,(CHARTYPE *)"",FALSE);
            rc = RC_TARGET_NOT_FOUND;
         }
         else
         {
            target->num_lines = MARK_VIEW->mark_end_line - MARK_VIEW->mark_start_line + 1L;
            target->true_line = MARK_VIEW->mark_start_line;
            target->last_line = MARK_VIEW->mark_end_line;
         }
         TRACE_RETURN();
         return(rc);
         break;
      case TARGET_BLOCK_CURRENT:
         if (MARK_VIEW == NULL)
         {
            if (allow_error_display)
               display_error(44,(CHARTYPE *)"",FALSE);
            rc = RC_TARGET_NOT_FOUND;
         }
         else
         {
            if (MARK_VIEW != CURRENT_VIEW)
            {
               if (allow_error_display)
                  display_error(45,(CHARTYPE *)"",FALSE);
               rc = RC_TARGET_NOT_FOUND;
            }
            else
            {
               target->num_lines = MARK_VIEW->mark_end_line - MARK_VIEW->mark_start_line + 1L;
               target->true_line = MARK_VIEW->mark_start_line;
               target->last_line = MARK_VIEW->mark_end_line;
            }
         }
         TRACE_RETURN();
         return(rc);
         break;
      default:
         break;
   }
   /*
    * All other targets are potentially repeating targets...
    */
   rc = RC_TARGET_NOT_FOUND;
   line_number = true_line;
   curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
   num_lines = 0L;
   while(1)
   {
      status = find_rtarget_target(curr,target,true_line,line_number,&num_lines);
      if (status != RC_TARGET_NOT_FOUND)
         break;
      /*
       * We can determine the direction of execution based on the first
       * target, as all targets must have the same direction to have reached
       * here.
       */
      if (target->rt[0].negative)
      {
         curr = curr->prev;
         line_number--;
      }
      else
      {
         curr = curr->next;
         line_number++;
      }
      if (curr == NULL)
         break;
   }
   if (status == RC_OK)
   {
      num_lines = ((target->rt[0].negative) ? -num_lines : num_lines);
      target->num_lines = num_lines;
      target->true_line = true_line;
      if (target->rt[0].negative)
      {
         curr = curr->next;
         target->last_line = find_next_in_scope(CURRENT_VIEW,curr,++line_number,DIRECTION_FORWARD);
      }
      else
      {
         curr = curr->prev;
         target->last_line = find_next_in_scope(CURRENT_VIEW,curr,--line_number,DIRECTION_BACKWARD);
      }
      rc = RC_OK;
   }
   else if (status == RC_TARGET_NOT_FOUND)
   {
      if (allow_error_display)
         display_error(17,target->string,FALSE);
      rc = RC_TARGET_NOT_FOUND;
   }
   else
      rc = status;
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short find_column_target(CHARTYPE *line,LENGTHTYPE len,TARGET *target,LENGTHTYPE true_column,bool display_parse_error,bool allow_error_display)
#else
short find_column_target(line,len,target,true_column,display_parse_error,allow_error_display)
CHARTYPE *line;
LENGTHTYPE len;
TARGET *target;
LENGTHTYPE true_column;
bool display_parse_error,allow_error_display;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   LINETYPE column_number=0L;
   LINETYPE num_columns=0L;
   bool status=FALSE;

   TRACE_FUNCTION("target.c:  find_column_target");
   /*
    * All column targets are potentially repeating targets...
    */
   rc = RC_TARGET_NOT_FOUND;
   status = FALSE;
   column_number = true_column;
   num_columns = 0;
   while(1)
   {
      status = find_rtarget_column_target(line,len,target,true_column,column_number,&num_columns);
      if (status)
         break;
      /*
       * We can determine the direction of execution based on the first
       * target, as all targets must have the same direction to have reached
       * here.
       */
      if (target->rt[0].negative)
      {
         if (column_number-- == (LINETYPE)CURRENT_VIEW->zone_start - 2L)
         {
            status = FALSE;
            break;
         }
      }
      else
      {
         if (column_number++ == (LINETYPE)CURRENT_VIEW->zone_end + 2L)
         {
            status = FALSE;
            break;
         }
      }
   }
   if (status)
   {
      num_columns = ((target->rt[0].negative) ? -num_columns : num_columns);
      target->num_lines = num_columns;
      target->true_line = (LINETYPE)true_column;
      target->last_line = (LINETYPE)column_number;
      rc = RC_OK;
   }
   else
   {
      if (allow_error_display)
         display_error(17,target->string,FALSE);
      rc = RC_TARGET_NOT_FOUND;
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static bool is_blank(LINE *curr)
#else
static bool is_blank(curr)
LINE *curr;
#endif
/***********************************************************************/
{
   register short i=0;
   bool rc=TRUE;

   TRACE_FUNCTION("target.c:  is_blank");
   if (CURRENT_VIEW->zone_start > curr->length)
   {
      TRACE_RETURN();
      return(TRUE);
   }
   for (i=CURRENT_VIEW->zone_start-1;i<min(CURRENT_VIEW->zone_end,curr->length);i++)
   {
      if (*(curr->line+i) != ' ')
      {
         rc = FALSE;
         break;
      }
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
LINE *find_named_line(CHARTYPE *name,LINETYPE *retline,bool respect_scope)
#else
LINE *find_named_line(name,retline,respect_scope)
CHARTYPE *name;
LINETYPE *retline;
bool respect_scope;
#endif
/***********************************************************************/
{
   LINETYPE lineno=0;
   LINE *curr=NULL;

   TRACE_FUNCTION("target.c:  find_named_line");
   /*
    * Find the line number in the current file of the named line specified
    */
   curr = CURRENT_FILE->first_line;
   while(curr != (LINE *)NULL)
   {
      /*
       * Check the line's name if we are not respecting scope or if we are
       * respecting scope and the line is in scope.
       */
      if (!respect_scope
      || (respect_scope && (IN_SCOPE(CURRENT_VIEW,curr) || CURRENT_VIEW->scope_all)))
      {
         if (curr->name != (CHARTYPE *)NULL)
         {
            if (strcmp((DEFCHAR *)curr->name,(DEFCHAR *)name) == 0)
            {
               TRACE_RETURN();
               *retline = lineno;
               TRACE_RETURN();
               return(curr);
            }
         }
      }
      lineno++;
      curr = curr->next;
   }
   TRACE_RETURN();
   return((LINE *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short find_string_target(LINE *curr,RTARGET *rt)
#else
short find_string_target(curr,rt)
LINE *curr;
RTARGET *rt;
#endif
/***********************************************************************/
{
   CHARTYPE *haystack=curr->line;
   CHARTYPE _THE_FAR temp_str[MAX_COMMAND_LENGTH];
   CHARTYPE *needle=temp_str;
   LENGTHTYPE needle_length=0,haystack_length=0,real_start=0,real_end=0;
   bool use_trec=FALSE;
   short rc=RC_OK;
   short loc=(-1);
   short str_length=0;

   TRACE_FUNCTION("target.c:  find_string_target");
   /*
    * Copy the supplied string target rather than point to it, as we don't
    * want to change the value of the target if it is a HEX string.
    */
   strcpy((DEFCHAR*)temp_str,(DEFCHAR*)rt->string);
   /*
    * If HEX is on, convert the target from a HEX format to CHARTYPE.
    */
   if (CURRENT_VIEW->hex == TRUE)
   {
      rc = convert_hex_strings(needle);
      if (rc == (-1))
      {
         display_error(32,needle,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      else
         needle_length = rc;
   }
   else
      needle_length = strlen((DEFCHAR *)needle);
   /*
    * Set the length of the string to be the actual length
    * of the string target.
    */
   rt->length = needle_length;
   /*
    * Determine if we need to copy the contents of the line into trec.
    * The reasons we need to do this are:
    * - the length of the needle is 0
    * - the last character of needle is a space
    */
   if ( needle_length == 0 )
      use_trec = TRUE;
   else
   {
      if (*(needle+(needle_length-1)) == ' ')
         use_trec = TRUE;
   }
   if (use_trec)
   {
      memset(trec,' ',max_line_length);
      memcpy(trec,curr->line,curr->length);
      haystack = trec;
/*    haystack_length = max_line_length;*/
      haystack_length = min(max_line_length,max(CURRENT_VIEW->zone_start,curr->length)+needle_length);
   }
   else
   {
      haystack = curr->line;
      haystack_length = curr->length;
   }
   /*
    * Calculate the bounds to search in based on length of haystack and
    * ZONE settings. If the haystack is empty, no need to search.
    */
   if (haystack_length != 0)
   {
      real_end = min(haystack_length-1,CURRENT_VIEW->zone_end-1);
      real_start = max(0,CURRENT_VIEW->zone_start-1);
      /*
       * Find the needle in the haystack.
       */
      loc = memfind(haystack+real_start,needle,(real_end-real_start+1),
                  needle_length,(CURRENT_VIEW->case_locate == CASE_IGNORE) ? TRUE : FALSE,
                  CURRENT_VIEW->arbchar_status,
                  CURRENT_VIEW->arbchar_single,
                  CURRENT_VIEW->arbchar_multiple,
                  &str_length);
    }
    if (loc == (-1))
       rc = RC_TARGET_NOT_FOUND;
    else
    {
       rt->start = loc;
       rt->found_length = str_length;
       rc = RC_OK;
    }

    TRACE_RETURN();
    return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short find_regexp(LINE *curr,RTARGET *rt)
#else
short find_regexp(curr,rt)
LINE *curr;
RTARGET *rt;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   CHARTYPE *haystack=NULL;
   LENGTHTYPE len,i,haystack_length=0,real_start=0,real_end=0;
   short rc=RC_TARGET_NOT_FOUND;
   long re_len;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("target.c:  find_regexp");
   /*
    * Search for the compiled RE
    */

   haystack = curr->line;
   haystack_length = curr->length;
   /*
    * Calculate the bounds to search in based on length of haystack and
    * ZONE settings. If the haystack is empty, no need to search.
    */
   if (haystack_length == 0)
   {
      haystack = (CHARTYPE *)"";
      real_end = real_start = 0;
      len = 0;
   }
   else
   {
      real_end = min(haystack_length-1,CURRENT_VIEW->zone_end-1);
      real_start = max(0,CURRENT_VIEW->zone_start-1);
      len = real_end - real_start + 1;
   }

   for (i=0;i<len;)
   {
      re_len = re_match( &rt->pattern_buffer, (DEFCHAR *)haystack+i,len-i,0,0);
      if ( re_len > 0 )
      {
         rt->length = re_len;
         rt->start = real_start+i; /* ?? */
         rc = RC_OK;
         break;
      }
      else
         i++;
   }

   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short find_rtarget_target(LINE *curr,TARGET *target,LINETYPE true_line,LINETYPE line_number,LINETYPE *num_lines)
#else
short find_rtarget_target(curr,target,true_line,line_number,num_lines)
LINE *curr;
TARGET *target;
LINETYPE true_line,line_number;
LINETYPE *num_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register short i=0;
   bool target_found=FALSE,status=FALSE;
   LINETYPE multiplier=0;
   short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("target.c:  find_rtarget_target");
   /*
    * If the line is not in scope and scope is respected, return FALSE.
    */
   if (!(IN_SCOPE(CURRENT_VIEW,curr))
   &&  !target->ignore_scope)
   {
      if (!CURRENT_VIEW->scope_all
      && !TOF(line_number)
      && !BOF(line_number))
      {
         TRACE_RETURN();
         return(RC_TARGET_NOT_FOUND);
      }
   }
   if (line_number != true_line)
      *num_lines = *num_lines + 1L;
   for (i=0;i<target->num_targets-((target->spare == (-1)) ? 0 : 1);i++)
   {
      target_found = FALSE;
      multiplier = (target->rt[i].negative) ? -1L : 1L;
      switch(target->rt[i].target_type)
      {
         case TARGET_BLANK:
            if (true_line == line_number)
            {
               target_found = ((target->rt[i].not) ? TRUE : FALSE);
               break;
            }
            if (target->rt[0].negative)
            {
               if (curr->prev == NULL)
                  break;
            }
            else
            {
               if (curr->next == NULL)
                  break;
            }
            target_found = is_blank(curr);
            break;
         case TARGET_POINT:
            if (curr->name == (CHARTYPE *)NULL)
               break;
            if (strcmp((DEFCHAR *)curr->name,(DEFCHAR *)target->rt[i].string) == 0)
               target_found = TRUE;
            break;
         case TARGET_STRING:
            if (true_line == line_number)
            {
               target_found = ((target->rt[i].not) ? TRUE : FALSE);
               break;
            }
            if (target->rt[0].negative)
            {
               if (curr->prev == NULL)
                  break;
            }
            else
            {
               if (curr->next == NULL)
                  break;
            }
            rc = find_string_target(curr,&target->rt[i]);
            switch(rc)
            {
               case RC_OK:
                  target->rt[i].found = target_found = TRUE;
                  break;
               case RC_TARGET_NOT_FOUND:
                  break;
               default:
                  TRACE_RETURN();
                  return(rc);
                  break;
            }
            break;
         case TARGET_RELATIVE:
            if ((*num_lines * multiplier) == target->rt[i].numeric_target)
                  target_found = TRUE;
            if (target->rt[0].negative)
            {
               if (curr->prev == NULL)
               {
                  target_found = TRUE;
                  break;
               }
            }
            else
            {
               if (curr->next == NULL)
               {
                  target_found = TRUE;
                  break;
               }
            }
            break;
         case TARGET_ABSOLUTE:
            if (line_number == target->rt[i].numeric_target)
                  target_found = TRUE;
            break;
         case TARGET_REGEXP:
            if (true_line == line_number)
            {
               target_found = ((target->rt[i].not) ? TRUE : FALSE);
               break;
            }
            if (target->rt[0].negative)
            {
               if (curr->prev == NULL)
                  break;
            }
            else
            {
               if (curr->next == NULL)
                  break;
            }
            rc = find_regexp(curr,&target->rt[i]);
            switch(rc)
            {
               case RC_OK:
                  target->rt[i].found = target_found = TRUE;
                  break;
               case RC_TARGET_NOT_FOUND:
                  break;
               default:
                  TRACE_RETURN();
                  return(rc);
                  break;
            }
            break;
         default:
            break;
      }
      if (target->rt[i].not)
         target->rt[i].found = target_found = (target_found) ? FALSE : TRUE;
      switch(target->rt[i].boolean)
      {
         case ' ':
            status = target_found;
            break;
         case '&':
            status &= target_found;
            break;
         case '|':
            status |= target_found;
            break;
      }
   }
   TRACE_RETURN();
   return((status)?RC_OK:RC_TARGET_NOT_FOUND);
}
/***********************************************************************/
#ifdef HAVE_PROTO
bool find_rtarget_column_target(CHARTYPE *line,LENGTHTYPE len,TARGET *target,LENGTHTYPE true_column,LENGTHTYPE column_number,LINETYPE *num_columns)
#else
bool find_rtarget_column_target(line,len,target,true_column,column_number,num_columns)
CHARTYPE *line;
LENGTHTYPE len;
TARGET *target;
LENGTHTYPE true_column,column_number;
LINETYPE *num_columns;
#endif
/***********************************************************************/
{
   register short i=0;
   bool target_found=FALSE,status=FALSE;
   LINETYPE multiplier=0;
   LINE curr;

   TRACE_FUNCTION("target.c:  find_rtarget_column_target");
   if (column_number != true_column)
      *num_columns = *num_columns + 1L;
   for (i=0;i<target->num_targets-((target->spare == (-1)) ? 0 : 1);i++)
   {
      target_found = FALSE;
      multiplier = (target->rt[i].negative) ? -1L : 1L;
      switch(target->rt[i].target_type)
      {
         case TARGET_BLANK:
            if (true_column == column_number)
            {
               target_found = ((target->rt[i].not) ? TRUE : FALSE);
               break;
            }
            if (column_number < CURRENT_VIEW->zone_start
            ||  column_number > CURRENT_VIEW->zone_end)
            {
                target_found = FALSE;
                break;
            }
            if (column_number > len)
            {
               target_found = TRUE;
               break;
            }
            if (*(line+column_number-1) == ' ') /* should be blank word */
               target_found = TRUE;
            else
               target_found = FALSE;
            break;
         case TARGET_STRING:
#if 0
            if (true_line == line_number)
            {
               target_found = ((target->rt[i].not) ? TRUE : FALSE);
               break;
            }
            if (target->rt[0].negative)
            {
               if (curr->prev == NULL)
                  break;
            }
            else
            {
               if (curr->next == NULL)
                  break;
            }
#endif
            /*
             * We need to determine if the string target starts in the
             * column; column_number.
             */
            curr.line = line;
            curr.length = len;

            if (find_string_target(&curr,&target->rt[i]) == RC_OK
            && target->rt[i].start+1 == column_number )
               target_found = TRUE;
            break;
         case TARGET_ABSOLUTE:
            if (column_number == target->rt[i].numeric_target)
               target_found = TRUE;
            break;
         case TARGET_RELATIVE:
            if ((*num_columns * multiplier) == target->rt[i].numeric_target)
               target_found = TRUE;
            break;
         default:
            break;
      }
      if (target->rt[i].not)
         target_found = (target_found) ? FALSE : TRUE;
      switch(target->rt[i].boolean)
      {
         case ' ':
            status = target_found;
            break;
         case '&':
            status &= target_found;
            break;
         case '|':
            status |= target_found;
            break;
      }
   }
   TRACE_RETURN();
   return(status);
}
/***********************************************************************/
#ifdef HAVE_PROTO
LINETYPE find_next_in_scope(VIEW_DETAILS *view,LINE *in_curr,LINETYPE line_number,short direction)
#else
LINETYPE find_next_in_scope(view,in_curr,line_number,direction)
VIEW_DETAILS *view;
LINE *in_curr;
LINETYPE line_number;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINE *curr=in_curr;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("target.c:  find_next_in_scope");
 if (in_curr == NULL)
     curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,line_number,CURRENT_FILE->number_lines);
 for (;;line_number+=(LINETYPE)direction)
   {
    if (IN_SCOPE(view,curr))
       break;
    if (direction == DIRECTION_FORWARD)
       curr = curr->next;
    else
       curr = curr->prev;
    if (curr == NULL)
       break;
   }
 TRACE_RETURN();
 return(line_number);
}
/***********************************************************************/
#ifdef HAVE_PROTO
LINETYPE find_last_not_in_scope(VIEW_DETAILS *view,LINE *in_curr,LINETYPE line_number,short direction)
#else
LINETYPE find_last_not_in_scope(view,in_curr,line_number,direction)
VIEW_DETAILS *view;
LINE *in_curr;
LINETYPE line_number;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINE *curr=in_curr;
 LINETYPE offset=0L;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("target.c:  find_last_not_in_scope");
 if (in_curr == NULL)
     curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,line_number,CURRENT_FILE->number_lines);
 for (;;line_number+=(LINETYPE)direction)
   {
    if (IN_SCOPE(view,curr))
       break;
    if (direction == DIRECTION_FORWARD)
      {
       curr = curr->next;
       offset = (-1L);
      }
    else
      {
       curr = curr->prev;
       offset = 1L;
      }
    if (curr == NULL)
       break;
   }
 TRACE_RETURN();
 return(line_number+offset);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short validate_target(CHARTYPE *string,TARGET *target,short target_type,LINETYPE true_line,bool display_parse_error,bool allow_error_display)
#else
short validate_target(string,target,target_type,true_line,display_parse_error,allow_error_display)
CHARTYPE *string;
TARGET *target;
short target_type;
LINETYPE true_line;
bool display_parse_error,allow_error_display;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("target.c:  validate_target");
   rc = parse_target(string,true_line,target,target_type,display_parse_error,allow_error_display,FALSE);
   if (rc != RC_OK)
   {
      TRACE_RETURN();
      return(rc);
   }
   rc = find_target(target,true_line,display_parse_error,allow_error_display);
   if (rc != RC_OK)
   {
      TRACE_RETURN();
      return(RC_TARGET_NOT_FOUND);
   }
   TRACE_RETURN();
   return(RC_OK);
}
#ifdef NOT_USED_ANYMORE
/***********************************************************************/
#ifdef HAVE_PROTO
bool in_scope(VIEW_DETAILS *view,LINE *curr)
#else
bool in_scope(view,curr)
VIEW_DETAILS *view;
LINE *curr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 bool rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("target.c:  in_scope");
 if (curr->select < view->display_low
 ||  curr->select > view->display_high)
    rc = FALSE;
 else
    rc = TRUE;
 TRACE_RETURN();
 return(rc);
}
#endif
/***********************************************************************/
#ifdef HAVE_PROTO
void calculate_scroll_values(short *number_focus_rows,LINETYPE *new_focus_line,
                             LINETYPE *new_current_line,bool *limit_of_screen,
                             bool *limit_of_file,bool *leave_cursor,
                             short direction)
#else
void calculate_scroll_values(number_focus_rows,new_focus_line,new_current_line,
                             limit_of_screen,limit_of_file,leave_cursor,direction)
short *number_focus_rows;
LINETYPE *new_focus_line,*new_current_line;
bool *limit_of_screen,*limit_of_file,*leave_cursor;
short direction;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 unsigned short y=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("target.c:  calculate_scroll_values");
 *limit_of_screen = *limit_of_file = FALSE;
 *number_focus_rows = 0;
 *new_focus_line = (-1L);
 y = CURRENT_SCREEN.rows[WINDOW_FILEAREA];
 switch(direction)
   {
    case DIRECTION_FORWARD:
/*---------------------------------------------------------------------*/
/* Determine the new focus line and the number of rows to adjust the   */
/* cursor position.                                                    */
/*---------------------------------------------------------------------*/
         for (i=0;i<CURRENT_SCREEN.rows[WINDOW_FILEAREA];i++)
           {
            if (CURRENT_SCREEN.sl[i].line_number == CURRENT_VIEW->focus_line)
              {
               y = i;
               continue;
              }
            if (CURRENT_SCREEN.sl[i].line_number != (-1L)
            && y != CURRENT_SCREEN.rows[WINDOW_FILEAREA])
              {
               *number_focus_rows = i-y;
               *new_focus_line = CURRENT_SCREEN.sl[i].line_number;
               break;
              }
           }
/*---------------------------------------------------------------------*/
/* If we have NOT set a new focus line (because we are on the bottom   */
/* of the screen) the new focus line is the next line in scope (if     */
/* SHADOW is OFF). If SHADOW is ON, the new focus line is determined by*/
/* the status of the current focus line.                               */
/*---------------------------------------------------------------------*/
         if (*new_focus_line == (-1L))
           {
            if (CURRENT_VIEW->shadow)
              {
               *new_focus_line = CURRENT_SCREEN.sl[y].line_number +
                                 ((CURRENT_SCREEN.sl[y].number_lines_excluded == 0) ?
                                 1L :
                                 (LINETYPE)CURRENT_SCREEN.sl[y].number_lines_excluded);
              }
            else
              {
               if (CURRENT_SCREEN.sl[y].current->next != NULL)
                  *new_focus_line = find_next_in_scope(CURRENT_VIEW,CURRENT_SCREEN.sl[y].current->next,
                                   CURRENT_SCREEN.sl[y].line_number+1L,direction);
              }
           }
/*---------------------------------------------------------------------*/
/* Determine the new current line and the number of rows to adjust the */
/* cursor position.                                                    */
/*---------------------------------------------------------------------*/
         *leave_cursor = TRUE;
         *new_current_line = (-1L);
         for (i=CURRENT_VIEW->current_row+1;i<CURRENT_SCREEN.rows[WINDOW_FILEAREA];i++)
           {
            if (CURRENT_SCREEN.sl[i].line_type == LINE_LINE
            ||  CURRENT_SCREEN.sl[i].line_type == LINE_TOF   /* MH12 */
            ||  CURRENT_SCREEN.sl[i].line_type == LINE_EOF)  /* MH12 */
              {
               *new_current_line = CURRENT_SCREEN.sl[i].line_number;
               break;
              }
            if (CURRENT_SCREEN.sl[i].line_type == LINE_SHADOW)
               *leave_cursor = FALSE;
           }
/*---------------------------------------------------------------------*/
/* If we have NOT set a new current line (only way this can happen is  */
/* if all lines after the current line are RESERVED, SCALE or TABLINE) */
/* and the cursor is on the current line) the new current line is the  */
/* next line in scope.                                                 */
/*---------------------------------------------------------------------*/
         if (*new_current_line == (-1L))
           {
            if (CURRENT_SCREEN.sl[y].current->next != NULL)
               *new_current_line = find_next_in_scope(CURRENT_VIEW,CURRENT_SCREEN.sl[y].current->next,
                                   CURRENT_SCREEN.sl[y].line_number+1L,direction);
           }
/*---------------------------------------------------------------------*/
/* Set flags for bottom_of_screen and bottom_of_file as appropriate.   */
/*---------------------------------------------------------------------*/
         if (*number_focus_rows == 0)
            *limit_of_screen = TRUE;
/*         if (CURRENT_SCREEN.sl[y].line_type == LINE_TOF_EOF MH12 */
/*         &&  CURRENT_SCREEN.sl[y].current->next == NULL) MH12 */
         if (CURRENT_SCREEN.sl[y].line_type == LINE_EOF)
            *limit_of_file = TRUE;
         break;
    case DIRECTION_BACKWARD:
/*---------------------------------------------------------------------*/
/* Determine the new focus line and the number of rows to adjust the   */
/* cursor position.                                                    */
/*---------------------------------------------------------------------*/
         for (i=CURRENT_SCREEN.rows[WINDOW_FILEAREA]-1;i>-1;i--)
           {
            if (CURRENT_SCREEN.sl[i].line_number == CURRENT_VIEW->focus_line)
              {
               y = i;
               continue;
              }
            if (CURRENT_SCREEN.sl[i].line_number != (-1L)
            && y != CURRENT_SCREEN.rows[WINDOW_FILEAREA])
              {
               *number_focus_rows = y-i;
               *new_focus_line = CURRENT_SCREEN.sl[i].line_number;
               break;
              }
           }
/*---------------------------------------------------------------------*/
/* If we have NOT set a new focus line (because we are on the top      */
/* of the screen) the new focus line is the prev line in scope (if     */
/* SHADOW is OFF). If SHADOW is ON, the new focus line is determined by*/
/* the status of the current focus line.                               */
/*---------------------------------------------------------------------*/
         if (*new_focus_line == (-1L))
           {
            if (CURRENT_VIEW->shadow)
              {
               if (CURRENT_SCREEN.sl[y].line_type == LINE_SHADOW)
                  *new_focus_line = CURRENT_SCREEN.sl[y].line_number - 1L;
               else
                 {
                  if (CURRENT_SCREEN.sl[y].current->prev != NULL)
                    {
                     *new_focus_line = find_next_in_scope(CURRENT_VIEW,CURRENT_SCREEN.sl[y].current->prev,
                                       CURRENT_SCREEN.sl[y].line_number-1L,direction);
                     if (*new_focus_line != CURRENT_SCREEN.sl[y].line_number-1L)
                        *new_focus_line = *new_focus_line + 1;
                    }
                 }
              }
            else
              {
               if (CURRENT_SCREEN.sl[y].current->prev != NULL)
                  *new_focus_line = find_next_in_scope(CURRENT_VIEW,CURRENT_SCREEN.sl[y].current->prev,
                                   CURRENT_SCREEN.sl[y].line_number-1L,direction);
              }
           }
/*---------------------------------------------------------------------*/
/* Determine the new current line and the number of rows to adjust the */
/* cursor position.                                                    */
/*---------------------------------------------------------------------*/
         *leave_cursor = TRUE;
         *new_current_line = (-1L);
         for (i=CURRENT_VIEW->current_row-1;i>-1;i--)
           {
            if (CURRENT_SCREEN.sl[i].line_type == LINE_LINE
            ||  CURRENT_SCREEN.sl[i].line_type == LINE_TOF /* MH12 */
            ||  CURRENT_SCREEN.sl[i].line_type == LINE_EOF) /* MH12 */
              {
               *new_current_line = CURRENT_SCREEN.sl[i].line_number;
               break;
              }
            if (CURRENT_SCREEN.sl[i].line_type == LINE_SHADOW)
               *leave_cursor = FALSE;
           }
/*---------------------------------------------------------------------*/
/* If we have NOT set a new current line (only way this can happen is  */
/* if all lines before the current line are RESERVED, SCALE or TABLINE)*/
/* and the cursor is on the current line) the new current line is the  */
/* previous line in scope.                                             */
/*---------------------------------------------------------------------*/
         if (*new_current_line == (-1L))
           {
            if (CURRENT_SCREEN.sl[y].current->prev != NULL)
               *new_current_line = find_next_in_scope(CURRENT_VIEW,CURRENT_SCREEN.sl[y].current->prev,
                                  CURRENT_SCREEN.sl[y].line_number-1L,direction);
           }
/*---------------------------------------------------------------------*/
/* Set flags for top_of_screen and top_of_file as appropriate.         */
/*---------------------------------------------------------------------*/
         if (*number_focus_rows == 0)
            *limit_of_screen = TRUE;
/*         if (CURRENT_SCREEN.sl[y].line_type == LINE_TOF_EOF MH12 */
/*         &&  CURRENT_SCREEN.sl[y].current->prev == NULL) MH12 */
         if (CURRENT_SCREEN.sl[y].line_type == LINE_TOF)
            *limit_of_file = TRUE;
         break;
   }
 TRACE_RETURN();
 return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short find_last_focus_line(unsigned short *newrow)
#else
short find_last_focus_line(newrow)
unsigned short *newrow;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short row=(-1);
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("target.c:  find_last_focus_line");
 for (i=CURRENT_SCREEN.rows[WINDOW_FILEAREA]-1;i>-1;i--)
   {
    if (CURRENT_SCREEN.sl[i].line_number != (-1L))
      {
       *newrow = row = i;
       break;
      }
   }
 if (row == (-1))
    rc = RC_INVALID_OPERAND;
 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short find_first_focus_line(unsigned short *newrow)
#else
short find_first_focus_line(newrow)
unsigned short *newrow;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short row=(-1);
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("target.c:  find_first_focus_line");
 for (i=0;i<CURRENT_SCREEN.rows[WINDOW_FILEAREA];i++)
   {
    if (CURRENT_SCREEN.sl[i].line_number != (-1L))
      {
       *newrow = row = i;
       break;
      }
   }
 if (row == (-1))
    rc = RC_INVALID_OPERAND;
 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE find_unique_char(CHARTYPE *str)
#else
CHARTYPE find_unique_char(str)
CHARTYPE *str;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("target.c:  find_unique_char");
 for (i=254;i>0;i--)
   {
    if (strzeq(str,(CHARTYPE)i) == (-1))
      {
       TRACE_RETURN();
       return((CHARTYPE)i);
      }
   }
 TRACE_RETURN();
 return(0);
}
