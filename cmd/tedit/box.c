/***********************************************************************/
/* BOX.C -                                                             */
/* This file contains all functions relating to box operations.        */
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

static char RCSid[] = "$Id: box.c,v 1.5 2001/12/18 08:23:26 mark Exp $";

#include <the.h>
#include <proto.h>

/* structure for passing box parameters */
struct _boxp
{
   short action;
   short mark_type;
   LENGTHTYPE src_start_col;
   LENGTHTYPE dst_start_col;
   LENGTHTYPE src_end_col;
   LENGTHTYPE num_cols;
   LINETYPE src_start_line;
   LINETYPE dst_start_line;
   LINETYPE src_end_line;
   LINETYPE num_lines;
   LINE *curr_src;
   LINE *curr_dst;
   VIEW_DETAILS *src_view;
   VIEW_DETAILS *dst_view;
};
typedef struct _boxp BOXP;

#ifdef HAVE_PROTO
static short box_copy_to_temp(BOXP *);
static short box_copy_from_temp(BOXP *,bool);
static short box_copy_stream_from_temp(BOXP *,bool);
static short box_delete(BOXP *);
static short box_move(BOXP *,bool);
static short box_fill(BOXP *,CHARTYPE);
#else
static short box_copy_to_temp();
static short box_copy_from_temp();
static short box_copy_stream_from_temp();
static short box_delete();
static short box_move();
static short box_fill();
#endif

/*#define DEBUG 1*/
/***********************************************************************/
#ifdef HAVE_PROTO
void box_operations(short action,CHARTYPE reset,bool boverlay,CHARTYPE fillchar)
#else
void box_operations(action,reset,boverlay,fillchar)
short action;
CHARTYPE reset;
bool boverlay;
CHARTYPE fillchar;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   BOXP boxp;
   short rc=RC_OK;
   unsigned short y=0,x=0;
   LENGTHTYPE offset=0;
   short save_mark_type=MARK_VIEW->mark_type;
   bool same_view=FALSE;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("box.c:     box_operations");
   /*
    * This procedure is for copying, deleting, filling, moving and
    * overlaying box blocks. Box blocks consist of BOX, WORD and COLUMN
    * blocks.
    */
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   /*
    * If the command was issued on the command line then the destination
    * line is the current line and the destination column is 0;
    */
   if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
   {
      if (CURRENT_VIEW->current_line == 0L)
         boxp.dst_start_line = 1L;
      else
         boxp.dst_start_line = CURRENT_VIEW->current_line;
      boxp.dst_start_col = 0;
   }
   else
   {
      if (CURRENT_VIEW->focus_line == 0L)
         boxp.dst_start_line = 1L;
      else
         boxp.dst_start_line = CURRENT_VIEW->focus_line;
      getyx(CURRENT_WINDOW,y,x);
      boxp.dst_start_col = x + CURRENT_VIEW->verify_col-1;
   }
   /*
    * Set up the source and destination views...
    */
   boxp.src_view = MARK_VIEW;
   boxp.dst_view = CURRENT_VIEW;
   if ( MARK_VIEW == CURRENT_VIEW )
      same_view = TRUE;

   boxp.mark_type = MARK_VIEW->mark_type;

   /*
    * CUA block can have start position after end position
    */
   if ( boxp.mark_type == M_CUA
   && ( MARK_VIEW->mark_start_line * max_line_length ) + MARK_VIEW->mark_start_col > (MARK_VIEW->mark_end_line * max_line_length ) + MARK_VIEW->mark_end_col )
   {
      boxp.src_start_line = MARK_VIEW->mark_end_line;
      boxp.src_start_col = MARK_VIEW->mark_end_col-1;
      boxp.src_end_line = MARK_VIEW->mark_start_line;
      boxp.src_end_col = MARK_VIEW->mark_start_col-1;
   }
   else
   {
      boxp.src_start_line = MARK_VIEW->mark_start_line;
      boxp.src_start_col = MARK_VIEW->mark_start_col-1;
      boxp.src_end_line = MARK_VIEW->mark_end_line;
      boxp.src_end_col = MARK_VIEW->mark_end_col-1;
   }

   boxp.num_cols =  MARK_VIEW->mark_end_col - MARK_VIEW->mark_start_col + 1;
   /*
    * If the block type is COLUMN, the number of lines to operate on is
    * the number of lines in the source file and the destination start
    * line is line 1. Reset these values set above.
    */
   if (MARK_VIEW->mark_type == M_COLUMN)
   {
      boxp.num_lines = MARK_VIEW->file_for_view->number_lines - boxp.src_start_line +1L;
      boxp.dst_start_line = 1L;
   }
   else
      boxp.num_lines = boxp.src_end_line - boxp.src_start_line +1L;
   
   /*
    * Find the current LINE pointer for both the source and destination
    * lines.
    */
   boxp.curr_src = lll_find(MARK_FILE->first_line,MARK_FILE->last_line,boxp.src_start_line,MARK_FILE->number_lines);
   if (action != BOX_D)
      boxp.curr_dst = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,boxp.dst_start_line,CURRENT_FILE->number_lines);
   /*
    * Call the appropriate box function...
    */
   boxp.action = action;
   switch(action)
   {
      case BOX_D:
         rc = box_delete(&boxp);
         break;
      case BOX_M:
         rc = box_move(&boxp,boverlay);
         break;
      case BOX_C:
         rc = box_move(&boxp,boverlay);
         break;
      case BOX_F:
         rc = box_fill(&boxp,fillchar);
         break;
   }
   /*
    * Increment the alteration counts...
    */
   increment_alt(CURRENT_FILE);
   /*
    * Set the parameters in the MARK_VIEW to OFF, and
    * redraw the view if different from the current view
    */
   MARK_VIEW->marked_line = MARK_VIEW->marked_col = FALSE;
   MARK_VIEW->mark_type = M_NONE;
   MARK_VIEW = (VIEW_DETAILS *)NULL;
   if ( !same_view
   &&   display_screens > 1)
   {
      build_screen(other_screen);
      display_screen(other_screen);
   }
   /*
    * If we are not resetting the block, set up block markers...
    */
   if (reset != SOURCE_BLOCK_RESET)
   {
      if (boxp.src_start_col < boxp.dst_start_col
      && action == BOX_M)
         offset = boxp.num_cols;
      MARK_VIEW = CURRENT_VIEW;
      MARK_VIEW->mark_start_line = boxp.dst_start_line;
      MARK_VIEW->mark_end_line = boxp.dst_start_line+boxp.num_lines-1L;
      MARK_VIEW->mark_start_col = boxp.dst_start_col+1-offset;
      if ( (save_mark_type == M_STREAM || save_mark_type == M_CUA )
      &&  boxp.num_lines != 1)
         MARK_VIEW->mark_end_col = boxp.src_end_col+1;
      else
         MARK_VIEW->mark_end_col = boxp.dst_start_col+boxp.num_cols-offset;
      MARK_VIEW->mark_type = save_mark_type;
      MARK_VIEW->marked_line = MARK_VIEW->marked_col = TRUE;
      wmove(CURRENT_WINDOW,y,x-offset);
   }
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   build_screen(current_screen); 
   display_screen(current_screen); /* should only call this is the marked block is in view */
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short box_delete(BOXP *prm)
#else
static short box_delete(prm)
BOXP *prm;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   LINETYPE i=0L;
   LENGTHTYPE j=0;
   LENGTHTYPE num_to_move=0,length_diff=0;
   short rc=0;
   bool shadow_found = FALSE;
   bool advance_line_ptr = TRUE;
   LINE *curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("box.c:     box_delete");
   curr = prm->curr_src;
   for (i=0L;i<prm->num_lines;i++)
   {
      advance_line_ptr = TRUE;
      rc = processable_line(prm->src_view,prm->src_start_line+i,curr);
      switch(rc)
      {
         case LINE_SHADOW:
            shadow_found = TRUE;
            break;
         case LINE_TOF:
         case LINE_EOF:
            break;
         default:
            if ( (prm->mark_type == M_STREAM || prm->mark_type == M_CUA )
            &&  prm->src_start_line != prm->src_end_line)
            {
               if (i == 0) 
               {
                  /* 
                   * first line of stream block 
                   * delete from start column to end of line
                   */
                  if (prm->src_start_col < curr->length)
                  {
                      add_to_recovery_list(curr->line,curr->length);
                      curr->length = prm->src_start_col;
                  }
               }
               else if (i+1 == prm->num_lines) /* last line of stream block */
               {
                  if (curr->length > 0)
                  {
                      add_to_recovery_list(curr->line,curr->length);
                      curr->length = (LENGTHTYPE)(prm->src_end_col < curr->length-1) ? curr->length - prm->src_end_col - 1: 0;
                      for (j=0;j<curr->length;j++)
                         *(curr->line+(LENGTHTYPE)(LINETYPE)j) = *(curr->line+prm->src_end_col+j+1);
                  }
               }
               else
               {
                  curr = delete_LINE(MARK_FILE->first_line,MARK_FILE->last_line,curr,DIRECTION_FORWARD);
                  MARK_FILE->number_lines--;
                  advance_line_ptr = FALSE;
               }
            }
            else
            {
               if ((LINETYPE)curr->length >= (LINETYPE)MARK_VIEW->mark_start_col)
               {
                  num_to_move = (LENGTHTYPE)max((LINETYPE)curr->length - (LINETYPE)MARK_VIEW->mark_end_col,0L);
                  length_diff = (LENGTHTYPE)min((LINETYPE)MARK_VIEW->mark_end_col-(LINETYPE)MARK_VIEW->mark_start_col+1L,
                                                (LINETYPE)curr->length-(LINETYPE)MARK_VIEW->mark_start_col+1L);
               }
               else
                  num_to_move = length_diff = 0;
               if (length_diff != 0)
               {
                  add_to_recovery_list(curr->line,curr->length);
                  curr->length = (LENGTHTYPE)((LINETYPE)curr->length-(LINETYPE)length_diff);
                  for (j=0;j<num_to_move;j++)
                     *(curr->line+(LENGTHTYPE)((LINETYPE)j+(LINETYPE)MARK_VIEW->mark_start_col-1L)) = *(curr->line+prm->src_start_col+j+prm->num_cols);
                  *(curr->line+curr->length) = '\0';/* null terminate */
               }
            }
      }
      if (advance_line_ptr)
         curr = curr->next;  /* this should NEVER go past the end */
   }
   /*
    * if the delete involved lines and there were no shadow lines involved
    * join the first line with the last line of the block
    */
   if ( (prm->mark_type == M_STREAM || prm->mark_type == M_CUA )
   &&  prm->src_start_line != prm->src_end_line
   &&  !shadow_found)
   {
      curr = prm->curr_src;
      if (curr->next->length > 0)
      {
         curr->line = (CHARTYPE *)(*the_realloc)((void *)curr->line,(curr->length+curr->next->length)*sizeof(CHARTYPE));
         if (curr->line == NULL)
         {
            display_error(30,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_OUT_OF_MEMORY);
         }
         memcpy((DEFCHAR*)curr->line+curr->length,(DEFCHAR*)curr->next->line,curr->next->length);
         curr->length += curr->next->length;
         if (curr->length > max_line_length)
         {
            display_error(0,(CHARTYPE*)"Truncated",FALSE);
            curr->length = max_line_length;
         }
         *(curr->line+curr->length) = '\0'; /* do we need to do this anymore ? */
      }
      curr = delete_LINE(MARK_FILE->first_line,MARK_FILE->last_line,curr->next,DIRECTION_FORWARD);
      MARK_FILE->number_lines--;
   }
   TRACE_RETURN();
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short box_move(BOXP *prm,bool boverlay)
#else
static short box_move(prm,boverlay)
BOXP *prm;
bool boverlay;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINE *first_save=NULL,*save_src=NULL,*temp_src=NULL;
 LINETYPE save_src_start_line=0L;
 LENGTHTYPE save_src_start_col=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("box.c:     box_move");
 save_src_start_col = prm->src_start_col;
 save_src_start_line = prm->src_start_line;
 save_src = prm->curr_src;
 box_copy_to_temp(prm);                  /* copy box from file to temp */
 if (prm->dst_start_col <= prm->src_start_col+prm->num_cols
 &&  prm->action == BOX_M)
   {
    temp_src = prm->curr_src; /* save the LINE pointer to temp src lines */
    prm->curr_src = save_src; /* point to file src lines */
    box_delete(prm);
    prm->curr_src = temp_src; /* point to temp src lines */
   }
 prm->src_start_line = 0L;
 prm->src_start_col = 0;
 if ( (prm->mark_type == M_STREAM || prm->mark_type == M_CUA )
 &&  prm->num_lines != 1)
    box_copy_stream_from_temp(prm,boverlay);/* copy box from temp to file */
 else
    box_copy_from_temp(prm,boverlay);    /* copy box from temp to file */
 first_save = lll_free(first_save);
 prm->src_start_line = save_src_start_line;
 prm->src_start_col = save_src_start_col;
 prm->curr_src = save_src;
 if (prm->dst_start_col > prm->src_start_col+prm->num_cols
 &&  prm->action == BOX_M)
    box_delete(prm);

 TRACE_RETURN();
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short box_copy_to_temp(BOXP *prm)
#else
static short box_copy_to_temp(prm)
BOXP *prm;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINE *first_save=NULL,*save_src=NULL,*tmp=NULL;
 LINETYPE i=0L;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("box.c:     box_copy_to_temp");
 tmp = prm->curr_src;
 for (i=0L;i<prm->num_lines;i++)
 {
    if (processable_line(prm->src_view,prm->src_start_line+i,tmp) == LINE_LINE)
    {
       memset(rec,' ',max_line_length);
#if 1
       if (prm->mark_type == M_STREAM
       ||  prm->mark_type == M_CUA)
       {
           int mystart=0,mynum=tmp->length;
           memset(trec,' ', max_line_length);
           memcpy(trec,tmp->line,tmp->length);
           if (i == 0)
           {
              mystart = prm->src_start_col;
              if (mystart > tmp->length)
                 mystart = mynum = 0;
              else
                 mynum  = tmp->length - mystart;
           }
           if (i+1 == prm->num_lines)
              mynum = prm->src_end_col - mystart + 1;
           memcpy(rec, trec+mystart,mynum);
           rec_len = mynum;
           if ((save_src = add_LINE(first_save,save_src,rec,rec_len,0,TRUE)) == (LINE *)NULL)
           {
              TRACE_RETURN();
              return(RC_OUT_OF_MEMORY);
           }
       }
       else
       {
          memcpy(rec,tmp->line,tmp->length);
          rec_len = tmp->length;
          if ((save_src = add_LINE(first_save,save_src,rec+prm->src_start_col,prm->num_cols,0,TRUE)) == (LINE *)NULL)
          {
             TRACE_RETURN();
             return(RC_OUT_OF_MEMORY);
          }
       }

#else
       memcpy(rec,tmp->line,tmp->length);
       rec_len = tmp->length;
       if ((save_src = add_LINE(first_save,save_src,
            rec+prm->src_start_col,prm->num_cols,0,TRUE)) == (LINE *)NULL)
       {
          TRACE_RETURN();
          return(RC_OUT_OF_MEMORY);
       }
#endif
       if (first_save == (LINE *)NULL)
          first_save = save_src;
    }
    tmp = tmp->next;
 }
 prm->curr_src = first_save;
 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short box_copy_from_temp(BOXP *prm,bool boverlay)
#else
static short box_copy_from_temp(prm,boverlay)
BOXP *prm;
bool boverlay;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE dst_lineno=0L;
 LENGTHTYPE j=0;
 CHARTYPE chr=0;
 short rc=RC_OK,line_type=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("box.c:     box_copy_from_temp");
 dst_lineno = prm->dst_start_line;
 while(prm->curr_src)
 {
    while(1)
    {
       line_type = processable_line(prm->dst_view,dst_lineno,prm->curr_dst);
       if (line_type == LINE_LINE)
          break;
/*       if (line_type == LINE_TOF_EOF) MH12 */
       if (line_type == LINE_TOF
       ||  line_type == LINE_EOF)
       {
          if ((prm->curr_dst = add_LINE(CURRENT_FILE->first_line,prm->curr_dst->prev,
                                 (CHARTYPE *)"",0,0,TRUE)) == (LINE *)NULL)
          {
             TRACE_RETURN();
             return(RC_OUT_OF_MEMORY);
          }
          CURRENT_FILE->number_lines++;
          break;
       }
       dst_lineno++;
       prm->curr_dst = prm->curr_dst->next;
    }

    pre_process_line(prm->dst_view,dst_lineno,prm->curr_dst);/* copy dest line into rec */
/* MH 300499    pre_process_line(prm->dst_view,dst_lineno,(LINE *)NULL); *//* copy dest line into rec */
    for (j=0;j<prm->num_cols;j++)
    {
       if (prm->src_start_col+j+1 > prm->curr_src->length)
          chr = (CHARTYPE)' ';
       else
          chr = (CHARTYPE)*(prm->curr_src->line+prm->src_start_col+j);
       if (boverlay)
          rec[prm->dst_start_col+j] = chr;
       else
          meminschr(rec,chr,prm->dst_start_col+j,max_line_length,rec_len++);
    }
    rc = memrevne(rec,' ',max_line_length);
    if (rc == (-1))
       rec_len = 0;
    else
       rec_len = rc+1;
    post_process_line(prm->dst_view,dst_lineno,(LINE *)NULL,FALSE);
    prm->curr_src = prm->curr_src->next;   /* this should NEVER go past the end */
    prm->curr_dst = prm->curr_dst->next;   /* this should NEVER go past the end */
    dst_lineno++;
 }
 prm->num_lines = dst_lineno - prm->dst_start_line;
 TRACE_RETURN();
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short box_copy_stream_from_temp(BOXP *prm,bool boverlay)
#else
static short box_copy_stream_from_temp(prm,boverlay)
BOXP *prm;
bool boverlay;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE dst_lineno=0L,i=0L;
 short line_type=0;
 int mystart=0,mynumcols=0;
 bool full_line=TRUE;
 LINE *curr=NULL,*last_line=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("box.c:     box_copy_stream_from_temp");
 if (boverlay)
 {
    /*
     * In overlay mode treat the stream block similar to a box block
     */
    dst_lineno = prm->dst_start_line;
    i = 0;
    while(prm->curr_src)
    {
       while(1)
       {
          line_type = processable_line(prm->dst_view,dst_lineno,prm->curr_dst);
          if (line_type == LINE_LINE)
          {
             break;
          }
/*       if (line_type == LINE_TOF_EOF) MH12 */
          if (line_type == LINE_TOF
          ||  line_type == LINE_EOF)
          {
             if ((prm->curr_dst = add_LINE(CURRENT_FILE->first_line,prm->curr_dst->prev,
                                    (CHARTYPE *)"",0,0,TRUE)) == (LINE *)NULL)
             {
                TRACE_RETURN();
                return(RC_OUT_OF_MEMORY);
             }
             CURRENT_FILE->number_lines++;
             break;
          }
          dst_lineno++;
          prm->curr_dst = prm->curr_dst->next;
       }
   
       pre_process_line(prm->dst_view,dst_lineno,prm->curr_dst);/* copy dest line into rec */
/* MH 300499    pre_process_line(prm->dst_view,dst_lineno,(LINE *)NULL); *//* copy dest line into rec */
       mystart=0;
       mynumcols=prm->curr_src->length;
       full_line=TRUE;
       if (i == 0)
       {
          mystart = prm->src_start_col;
          full_line = FALSE;
       }
       if (i+1 == prm->num_lines)
          full_line = FALSE;
       memcpy(rec+mystart,prm->curr_src->line,prm->curr_src->length);
       if (full_line)
       {
          rec_len = prm->curr_src->length;
       }
       else
       {
          rec_len = max(rec_len,prm->curr_src->length+mystart);
       }

       post_process_line(prm->dst_view,dst_lineno,(LINE *)NULL,FALSE);
       prm->curr_src = prm->curr_src->next;   /* this should NEVER go past the end */
       prm->curr_dst = prm->curr_dst->next;   /* this should NEVER go past the end */
       dst_lineno++;
       i++;
    }
    prm->num_lines = dst_lineno - prm->dst_start_line;
 }
 else
 {
    /*
     * In non-overlay mode treat the stream block similar to a line block
     */
    /*
     * This function copies saved lines from a multi-line stream block.
     * The line to which the stream block is copied is split at the
     * cursor position. 
     * The first source line is appended to the current line.
     * The remainder of the lines are added as new lines, except the last.
     * The last line is pre-pended to that portion of the line that was
     * split.
     */
    dst_lineno = prm->dst_start_line;
    while(1)
    {
       line_type = processable_line(prm->dst_view,dst_lineno,prm->curr_dst);
       if (line_type == LINE_LINE)
       {
          break;
       }
/*       if (line_type == LINE_TOF_EOF) MH12 */
       if (line_type == LINE_TOF
       ||  line_type == LINE_EOF)
       {
          if ((prm->curr_dst = add_LINE(CURRENT_FILE->first_line,prm->curr_dst->prev,
                                    (CHARTYPE *)"",0,0,TRUE)) == (LINE *)NULL)
          {
             TRACE_RETURN();
             return(RC_OUT_OF_MEMORY);
          }
          CURRENT_FILE->number_lines++;
          break;
       }
       dst_lineno++;
       prm->curr_dst = prm->curr_dst->next;
    }
    /*
     * Now have a line that we can split.
     */
    curr = prm->curr_dst;

    for (i=0;i<prm->num_lines;i++)
    {
       if (i == 0)
       {
          /*
           * Create the new line
           */
          memset(rec,' ',max_line_length);
          if (prm->dst_start_col < curr->length)
          {
            memcpy(rec,curr->line+prm->dst_start_col,curr->length-prm->dst_start_col);
            rec_len = curr->length-prm->dst_start_col;
          }
          else
            rec_len = 0;
          curr->flags.changed_flag = TRUE;
          /*
           * Add the new line
           */
          add_LINE(CURRENT_FILE->first_line,curr,(rec),rec_len,curr->select,TRUE);
          CURRENT_FILE->number_lines++;
          last_line = curr->next;
          /*
           * Get the current line back into rec so we can trim it at
           * the cursor position.
           */
          pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,curr);
          memset(rec+prm->dst_start_col,' ',max_line_length-prm->dst_start_col);
          memcpy(rec+prm->dst_start_col,prm->curr_src->line,min(max_line_length-prm->dst_start_col,prm->curr_src->length));
          rec_len = prm->curr_src->length+prm->dst_start_col;
          post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,curr,FALSE);
          prm->curr_src = prm->curr_src->next;   /* this should NEVER go past the end */
          curr = prm->curr_dst;
       }
       else if (i+1 == prm->num_lines)  /* last line - prepend to "last_line" */
       {
          pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,last_line);
          meminsmem(rec,prm->curr_src->line,prm->curr_src->length,
                      0,max_line_length,last_line->length);
          rec_len = min(max_line_length,rec_len+prm->curr_src->length);
          post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,last_line,FALSE);
       }
       else
       {
          if ((curr = add_LINE(CURRENT_FILE->first_line,curr,
                                    prm->curr_src->line,prm->curr_src->length,0,TRUE)) == (LINE *)NULL)
          {
             TRACE_RETURN();
             return(RC_OUT_OF_MEMORY);
          }
          prm->curr_src = prm->curr_src->next;   /* this should NEVER go past the end */
          CURRENT_FILE->number_lines++;
       }
    }
    pre_process_line(prm->dst_view,dst_lineno,prm->curr_dst);
 }
 TRACE_RETURN();
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short box_fill(BOXP *prm,CHARTYPE fillchar)
#else
static short box_fill(prm,fillchar)
BOXP *prm;
CHARTYPE fillchar;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   LINETYPE i=0L;
   short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("box.c:     box_fill");
   for (i=0L;i<prm->num_lines;i++)
   {
      if (processable_line(CURRENT_VIEW,prm->src_start_line+i,prm->curr_src) == LINE_LINE)
      {
         pre_process_line(CURRENT_VIEW,prm->src_start_line+i,(LINE *)NULL);/* copy source line into rec */
         if ( prm->mark_type == M_STREAM 
         ||   prm->mark_type == M_CUA )
         {
            int mystart=0,mynum=max_line_length;
            if (i == 0)
               mystart = prm->src_start_col;
            if (i+1 == prm->num_lines)
               mynum = prm->src_end_col - mystart + 1;
            memset(rec+mystart,fillchar,mynum);
         }
         else
            memset(rec+prm->src_start_col,fillchar,prm->num_cols);
         rc = memrevne(rec,' ',max_line_length);
         if (rc == (-1))
            rec_len = 0;
         else
            rec_len = rc+1;
         post_process_line(CURRENT_VIEW,prm->src_start_line+i,(LINE *)NULL,FALSE);
      }
      prm->curr_src = prm->curr_src->next;   /* this should NEVER go past the end */
   }

   prm->dst_start_col = prm->src_start_col;
   prm->dst_start_line = prm->src_start_line;

   TRACE_RETURN();
   return(RC_OK);
}
