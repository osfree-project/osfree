/***********************************************************************/
/* SORT.C - Functions related to the SORT command.                     */
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

static char RCSid[] = "$Id: sort.c,v 1.3 2001/12/18 08:23:27 mark Exp $";

#include <the.h>
#include <proto.h>

#define MAX_SORT_FIELDS 10

#define SF_ERROR    0
#define SF_START    1
#define SF_ORDER    2
#define SF_LEFT     3

struct sort_field
 {
  CHARTYPE order;                     /* A - ascending, D - descending */
  LENGTHTYPE left_col;                                  /* left column */
  LENGTHTYPE right_col;                                /* right column */
 };
typedef struct sort_field SORT_FIELD;

CHARTYPE *sort_field_1;
CHARTYPE *sort_field_2;

SORT_FIELD sort_fields[MAX_SORT_FIELDS];

short num_fields;

#ifdef __STDC__
static int cmp(const void *,const void *);
#else
static int cmp();
#endif

/***********************************************************************/
#ifdef __STDC__
static int cmp(const void *first,const void *second)
#else
static int cmp(first,second)
void *first,*second;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0,j=0;
 short len=0,rc=RC_OK;
 LENGTHTYPE right_col=0,left_col=0;
 _LINE *one = *(_LINE **)first;
 _LINE *two = *(_LINE **)second;
/*--------------------------- processing ------------------------------*/
/*---------------------------------------------------------------------*/
/* For each sort field defined in the array sort_fields, compare the   */
/* value of both lines for the specified column range.                 */
/*---------------------------------------------------------------------*/
 for (i=0;i<num_fields;i++)
    {
/*---------------------------------------------------------------------*/
/* Calculate the length of the sort field.                             */
/*---------------------------------------------------------------------*/
     len = sort_fields[i].right_col - sort_fields[i].left_col + 1;
/*---------------------------------------------------------------------*/
/* Set the two temporary fields to blanks.                             */
/*---------------------------------------------------------------------*/
     memset(sort_field_1,' ',len);
     memset(sort_field_2,' ',len);
/*---------------------------------------------------------------------*/
/* For the first line to be compared, extract the portion of the line  */
/* that corresponds with the current sort column...                    */
/*---------------------------------------------------------------------*/
     right_col = min(sort_fields[i].right_col,one->length);
     left_col = min(sort_fields[i].left_col,one->length);
/*---------------------------------------------------------------------*/
/* If the sort column lies after the end of the line, leave the        */
/* contents of the sort field blank.                                   */
/*---------------------------------------------------------------------*/
     if (sort_fields[i].left_col <= one->length)
        memcpy(sort_field_1,one->line+left_col-1,right_col-left_col+1);
/*---------------------------------------------------------------------*/
/* For the next  line to be compared, extract the portion of the line  */
/* that corresponds with the current sort column...                    */
/*---------------------------------------------------------------------*/
     right_col = min(sort_fields[i].right_col,two->length);
     left_col = min(sort_fields[i].left_col,two->length);
/*---------------------------------------------------------------------*/
/* If the sort column lies after the end of the line, leave the        */
/* contents of the sort field blank.                                   */
/*---------------------------------------------------------------------*/
     if (sort_fields[i].left_col <= two->length)
        memcpy(sort_field_2,two->line+left_col-1,right_col-left_col+1);
/*---------------------------------------------------------------------*/
/* If CASE IGNORE is on for the current view, set both sort fields to  */
/* uppercase for the comparison.                                       */
/*---------------------------------------------------------------------*/
     if (CURRENT_VIEW->case_sort == CASE_IGNORE)
       {
        for (j=0;j<len;j++)
          {
           if (islower(sort_field_1[j]))
              sort_field_1[j] = toupper(sort_field_1[j]);
           if (islower(sort_field_2[j]))
              sort_field_2[j] = toupper(sort_field_2[j]);
          }
       }
/*---------------------------------------------------------------------*/
/* If the two sort fields are equal, continue the sort with the next   */
/* sort field value. If the sort fields are different, return with the */
/* the comparison value (if ASCENDING) or the comparison value negated */
/* (if DESCENDING).                                                    */
/*---------------------------------------------------------------------*/
     if ((rc = strncmp((DEFCHAR *)sort_field_1,(DEFCHAR *)sort_field_2,len)) != 0)
        return((sort_fields[i].order == 'A') ? rc : -rc);
    }
/*---------------------------------------------------------------------*/
/* To get to here, the result of sorting on all fields has resulted in */
/* both lines being equal. Return with 0 to indicate this.             */
/*---------------------------------------------------------------------*/
 return(0);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short execute_sort(CHARTYPE *params)
#else
short execute_sort(params)
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define STATE_REAL   0
#define STATE_SHADOW 1
#define SOR_PARAMS  32
   register int i=0;
   CHARTYPE *word[SOR_PARAMS+1];
   CHARTYPE strip[SOR_PARAMS];
   unsigned short num_params=0;
   _LINE **lfirst=NULL,**lp=NULL;
   _LINE **origfirst=NULL,**origlp=NULL;
   _LINE *curr=NULL,*first=NULL,*last=NULL;
   _LINE *curr_prev=NULL,*curr_next=NULL;
   LINETYPE true_line=0L,dest_line=0L;
   LINETYPE abs_num_lines=0L;
   LINETYPE j=0L;
   LINETYPE num_actual_lines=0L;
   LINETYPE num_sorted_lines=0L,save_num_sorted_lines=0L;
   short rc=RC_OK,direction=DIRECTION_FORWARD;
   LENGTHTYPE left_col=0,right_col=0,max_column_width=0;
   CHARTYPE order='A';
   TARGET target;
   short target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL|TARGET_SPARE;
   bool lines_based_on_scope=FALSE;
   short state=STATE_REAL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("sort.c:    execute_sort");
   /*
    * Validate first argument as a target...
    */
   initialise_target(&target);
   rc = validate_target(params,&target,target_type,get_true_line(TRUE),TRUE,TRUE);
   if (rc != RC_OK)
   {
      free_target(&target);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   true_line = (target.num_lines < 0L)?target.last_line:target.true_line;
   direction = DIRECTION_FORWARD;
   abs_num_lines = ((target.num_lines < 0L) ? -target.num_lines : target.num_lines);
   lines_based_on_scope = (target.rt[0].target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
   /*
    * Don't need to do anything if < 2 lines to be sorted.
    */
   if (abs_num_lines < 2L)
   {
      free_target(&target);
      display_error(55,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * Parse the remainder of the arguments and set up the sort_fields[]
    * array with valid values.
    */
   if (target.spare != (-1))
   {
      for(i=0;i<SOR_PARAMS;i++)
         strip[i]=STRIP_BOTH;
      num_params = param_split(strtrunc(target.rt[target.spare].string),word,SOR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   }
   /*
    * Process parameters differently, depending on the number...
    * 0 parameter (target only) - if BOX BLOCK, then the sort field will
    *                             be the block columns, otherwise ZONE
    *                             settings will be used.
    * 1 parameters (target & order) - same as above but also validate the
    *                                 ordering value.
    * > 1 parameters (target & sort fields) - validate each parameter.
    */
   switch(num_params)
   {
      case 0:
      case 1:
         sort_fields[0].left_col = CURRENT_VIEW->zone_start;
         sort_fields[0].right_col = CURRENT_VIEW->zone_end;
         sort_fields[0].order = order;
         num_fields = 1;
         if (target.rt[0].target_type == TARGET_BLOCK_CURRENT
         && MARK_VIEW->mark_type == M_BOX)
         {
            sort_fields[0].left_col = MARK_VIEW->mark_start_col;
            sort_fields[0].right_col = MARK_VIEW->mark_end_col;
         }
         /*
          * No more processing if only 1 parameter.
          */
         if (num_params == 0)
            break;
         /*
          * Processing for 2 parameters; validate ordering value.
          */
         if (equal((CHARTYPE *)"ascending",word[0],1)
         ||  equal((CHARTYPE *)"descending",word[0],1))
         {
            order = word[0][0];
            if (islower(order))
               order = toupper(order);
            sort_fields[0].order = order;
            break;
         }
         /*
          * If the parameter is not Ascending or Descending, display error.
          */
         display_error(1,(CHARTYPE *)word[0],FALSE);
         free_target(&target);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
         break;
         /*
          * More than 1 parameters; sort field(s) are being specified.
          */
      default:
         i = 0;
         num_fields = 0;
         state = SF_START;
         while(1)
         {
            switch(state)
            {
               case SF_START:
                  if (equal((CHARTYPE *)"ascending",word[i],1)
                  ||  equal((CHARTYPE *)"descending",word[i],1))
                  {
                     order = word[i][0];
                     if (islower(order))
                        order = toupper(order);
                     sort_fields[num_fields].order = order;
                     state = SF_ORDER;
                     i++;
                     break;
                  }
                  left_col = atoi((DEFCHAR *)word[i]);
                  if (left_col == 0)
                  {
                     state = SF_ERROR;
                     break;
                  }
                  sort_fields[num_fields].order = order;
                  sort_fields[num_fields].left_col = left_col;
                  state = SF_LEFT;
                  i++;
                  break;
               case SF_ORDER:
                  left_col = atoi((DEFCHAR *)word[i]);
                  if (left_col < 1)
                  {
                     state = SF_ERROR;
                     break;
                  }
                  sort_fields[num_fields].left_col = left_col;
                  state = SF_LEFT;
                  i++;
                  break;
               case SF_LEFT:
                  right_col = atoi((DEFCHAR *)word[i]);
                  if (right_col < 1)
                  {
                     state = SF_ERROR;
                     break;
                  }
                  sort_fields[num_fields].right_col = right_col;
                  if (right_col < left_col)
                  {
                     state = SF_ERROR;
                     break;
                  }
                  state = SF_START;
                  i++;
                  num_fields++;
                  break;
               default:
                  state = SF_ERROR;
                  break;
            }
            /*
             * If we have an error, display a message...
             */
            if (state == SF_ERROR)
            {
               free_target(&target);
               display_error(1,(CHARTYPE *)word[i],FALSE);
               TRACE_RETURN();
               return(RC_INVALID_OPERAND);
            }
            /*
             * If we have run out of parameters...
             */
            if (i == num_params)
            {
               /*
                * ...then if we have the correct number of parameters, OK.
                */
               if (state == SF_START)
                  break;
               else
               /*
                * ...otherwise display an error.
                */
               {
                  free_target(&target);
                  display_error(1,strtrunc(target.rt[target.spare].string),FALSE);
                  TRACE_RETURN();
                  return(RC_INVALID_OPERAND);
               }
            }
         }
         break;
   }
   /*
    * Determine the maximum length of a sort field.
    */
   for (i=0;i<num_fields;i++)
      max_column_width = max(max_column_width,
                sort_fields[i].right_col - sort_fields[i].left_col + 1);
   /*
    * Allocate memory for each of the temporary sort fields to the length
    * of the maximum field width.
    */
   if ((sort_field_1 = (CHARTYPE *)(*the_malloc)(max_column_width)) == NULL)
   {
      free_target(&target);
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   if ((sort_field_2 = (CHARTYPE *)(*the_malloc)(max_column_width)) == NULL)
   {
      free_target(&target);
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   /*
    * Assign the values of the newly allocated array to the LINE pointers
    * for the target lines.
    */
   first = curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
   /*
    * Allocate memory for num_lines of LINE pointers and for a copy of
    * original lines.
    */
   if ((lfirst = (_LINE **)(*the_malloc)(abs_num_lines*sizeof(_LINE *))) == NULL)
   {
      free_target(&target);
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   if ((origfirst = (_LINE **)(*the_malloc)(3*abs_num_lines*sizeof(_LINE *))) == NULL)
   {
      free_target(&target);
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   lp = lfirst;
   origlp = origfirst;
   for (j=0L,num_actual_lines=0L;;j++)
   {
      if (lines_based_on_scope)
      {
         if (num_actual_lines == abs_num_lines)
            break;
      }
      else
      {
         if (abs_num_lines == j)
            break;
      }
      rc = processable_line(CURRENT_VIEW,true_line+(LINETYPE)(j*direction),curr);
      switch(rc)
      {
         case LINE_SHADOW:
            break;
         case LINE_TOF:
         case LINE_EOF:
            num_actual_lines++;
            break;
         default:
            lp[num_sorted_lines] = curr;
#if 0
            memcpy(origlp++,&curr,sizeof(_LINE*));
            memcpy(origlp++,&curr->next,sizeof(_LINE*));
            memcpy(origlp++,&curr->prev,sizeof(_LINE*));
#else
            *origlp++ = curr;
            *origlp++ = curr->next;
            *origlp++ = curr->prev;
#endif
            num_actual_lines++;
            num_sorted_lines++;
            break;
      }
      /*
       * Proceed to the next record, even if the current record not in scope.
       */
      if (direction == DIRECTION_BACKWARD)
        curr = curr->prev;
      else
         curr = curr->next;
      if (curr == NULL)
         break;
   }
   last = curr;
   save_num_sorted_lines = num_sorted_lines;
   origlp = origfirst;

   /*
    * Don't need to do anything if < 2 lines to be sorted.
    */
   if (num_sorted_lines < 2L)
   {
      display_error(55,(CHARTYPE *)"",FALSE);
   }
   else
   {
      /*
       * Sort the target array...
       */
      qsort(lfirst,num_sorted_lines,sizeof(_LINE *),cmp);
      /*
       * Merge  the sorted array pointers into the linked list...
       */
      lp = lfirst;
      origlp = origfirst;
      curr = first;
      for (j=0L,num_actual_lines=0L,num_sorted_lines=0L;;j++)
      {
         if (lines_based_on_scope)
         {
            if (num_actual_lines == abs_num_lines)
               break;
         }
         else
         {
            if (abs_num_lines == j)
               break;
         }
         rc = processable_line(CURRENT_VIEW,true_line+(LINETYPE)(j*direction),curr);
         switch(rc)
         {
            case LINE_SHADOW:
               curr_prev = curr->prev;
               curr_next = curr->next;
               if (state == STATE_REAL
               &&  num_sorted_lines !=0)
               {
                  lp[num_sorted_lines-1L]->next = origlp[((num_sorted_lines-1L)*3L)+1];
                  curr->prev = lp[num_sorted_lines-1L];
               }
               state = STATE_SHADOW;
               if (direction == DIRECTION_BACKWARD)
                  curr = curr_prev;
               else
                  curr = curr_next;
               break;
            case LINE_TOF:
            case LINE_EOF:
               num_actual_lines++;
               if (direction == DIRECTION_BACKWARD)
                  curr = curr->prev;
               else
                  curr = curr->next;
               break;
            default:
               if (num_sorted_lines == 0L)
               {
                  lp[num_sorted_lines]->next = lp[num_sorted_lines+1L];
                  lp[num_sorted_lines]->prev = origlp[(num_sorted_lines*3L)+2L];
                  lp[num_sorted_lines]->prev->next = lp[num_sorted_lines];
               }
               else
               {
                  if (num_sorted_lines == save_num_sorted_lines - 1L)
                  {
                     lp[num_sorted_lines]->next = origlp[(num_sorted_lines*3L)+1L];
                     lp[num_sorted_lines]->prev = lp[num_sorted_lines-1L];
                     lp[num_sorted_lines]->next->prev = lp[num_sorted_lines];
                  }
                  else
                  {
                     lp[num_sorted_lines]->next = lp[num_sorted_lines+1L];
                     lp[num_sorted_lines]->prev = lp[num_sorted_lines-1L];
                  }
               }
               if (state == STATE_SHADOW)
               {
                  lp[num_sorted_lines]->prev = origlp[(num_sorted_lines*3L)+2L];
                  lp[num_sorted_lines]->prev->next = lp[num_sorted_lines];
               }
               state = STATE_REAL;
               if (direction == DIRECTION_BACKWARD)
                  curr = origlp[(num_sorted_lines*3L)+2L];
               else
                  curr = origlp[(num_sorted_lines*3L)+1L];
               num_actual_lines++;
               num_sorted_lines++;
               break;
         }
         /*
          * Proceed to the next record, even if the current record not in scope.
          */
         if (curr == NULL)
            break;
      }
      /*
       * If STAY is OFF, change the current and focus lines by the number
       * of lines calculated from the target.
       */
      if (!CURRENT_VIEW->stay                                 /* stay is off */
      &&  abs_num_lines != 0L)
         CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW,NULL,dest_line,direction);
      /*
       * Increment alteration count...
       */
      increment_alt(CURRENT_FILE);

      sprintf((DEFCHAR *)temp_cmd,"%ld line(s) sorted",abs_num_lines);
      display_error(0,temp_cmd,TRUE);
   }

   /*
    * Free up the memory used for the sort fields and the target array.
    */
   (*the_free)(sort_field_1);
   (*the_free)(sort_field_2);
   (*the_free)(lfirst);
   (*the_free)(origfirst);
   free_target(&target);
   TRACE_RETURN();
   return(RC_OK);
}
