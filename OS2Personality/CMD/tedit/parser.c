/***********************************************************************/
/* PARSER.C - Functions that involve syntax highlighting               */
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
  
static char RCSid[] = "$Id: parser.c,v 1.19 2002/07/09 10:59:20 mark Exp $";
  
#include <the.h>
#include <proto.h>
#include <thematch.h>
  
#define STATE_START             0
#define STATE_SINGLE_QUOTE      1
#define STATE_DOUBLE_QUOTE      2
#define STATE_BACKSLASH_SINGLE  3
#define STATE_BACKSLASH_DOUBLE  4
#define STATE_IGNORE            5
#define STATE_CASE              6
#define STATE_OPTION            7
#define STATE_NUMBER            8
#define STATE_IDENTIFIER        9
#define STATE_STRING           10
#define STATE_COMMENT          11
#define STATE_HEADER           12
#define STATE_LABEL            13
#define STATE_MATCH            14
#define STATE_COLUMN           15
#define STATE_KEYWORD          16
#define STATE_POSTCOMPARE      17
#define STATE_FILE_HEADER      18
#define STATE_DELIMITER        19
#define STATE_FUNCTION         20
#define STATE_SPACE            21
#define STATE_SLASH            22
#define STATE_MARKUP           23
#define STATE_START_TAG        24
#define STATE_END_TAG          25

#define REXX_NUMBER_RE   "-?\\([0-9]+\\.\\|\\.?[0-9]\\)[0-9]*\\([eE][-+]?[0-9]+\\)?"
#define COBOL_NUMBER_RE  "-?\\([0-9]+\\.\\|\\.?[0-9]\\)[0-9]*"
#define C_NUMBER_RE      "-?\\(\\([0-9]+\\.\\|\\.?[0-9]\\)[0-9]*\\|[0][0-7]+\\|0[xX][0-9a-fA-F]+\\)\\([dDeE][-+]?[0-9]+\\)?[uU]?[lL]?"

static int number_blanks=0;
static CHARTYPE *work;
static char tmp[100];
  
/***********************************************************************/
#ifdef HAVE_PROTO
static long find_comment( FILE_DETAILS *fd, LENGTHTYPE start, LENGTHTYPE length, CHARTYPE *ptr, CHARTYPE *start_delim, CHARTYPE *end_delim, int *type )
#else
static long find_comment( fd, start, length, ptr, start_delim, end_delim, type)
FILE_DETAILS *fd;
LENGTHTYPE start, length;
CHARTYPE *ptr,*start_delim,*end_delim;
int *type;
#endif
/***********************************************************************/
{
   int i,j;
   CHARTYPE *ptr_start=start_delim,*ptr_end=end_delim;
   CHARTYPE ch1,ch2,ch3;

   TRACE_FUNCTION( "parser.c:  find_comment" );
   for ( i = start; i < length; i++ )
   {
      ptr_start = start_delim;
      if ( fd->parser->case_sensitive )
      {
         ch1 = ptr[i];
         ch2 = *ptr_start;
      }
      else
      {
         ch1 = tolower( ptr[i] );
         ch2 = tolower( *ptr_start );
      }
      if ( ch1 == ch2 )
      {
         for ( j = i; j < length; j++ )
         {
            ch2 = (fd->parser->case_sensitive) ? *ptr_start : tolower( *ptr_start );
            ch3 = (fd->parser->case_sensitive) ? ptr[j] : tolower( ptr[j] );
            if (ch2 == '\0')
            {
               *type = STATE_START_TAG;
               TRACE_RETURN();
               return i;
            }
            if ( ch3 != ch2 )
               break;
            ptr_start++;
         }
      }
      ptr_end = end_delim;
      if ( fd->parser->case_sensitive )
      {
         ch1 = ptr[i];
         ch2 = *ptr_end;
      }
      else
      {
         ch1 = tolower( ptr[i] );
         ch2 = tolower( *ptr_end );
      }
      if ( ch1 == ch2 )
      {
         for ( j = i; j < length; j++ )
         {
            ch2 = (fd->parser->case_sensitive) ? *ptr_end : tolower( *ptr_end );
            ch3 = (fd->parser->case_sensitive) ? ptr[j] : tolower( ptr[j] );
            if ( ch2 == '\0')
            {
               *type = STATE_END_TAG;
               TRACE_RETURN();
               return i;
            }
            if ( ch3 != ch2 )
               break;
            ptr_end++;
         }
      }
   }
   TRACE_RETURN();
   return -1;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static long find_single_comments( CHARTYPE scrno, FILE_DETAILS *fd, SHOW_LINE *scurr )
#else
static long find_single_comments( scrno, fd, scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
   PARSE_COMMENTS *curr=fd->parser->first_comments;
   LENGTHTYPE i,vcol,pos=0,len=scurr->length,len_comment;
   CHARTYPE *ptr;
   int state;
   chtype comment_colour;
   CHARTYPE ch1,ch2;

   TRACE_FUNCTION( "parser.c:  find_single_comments" );
   if ( scurr->is_current_line )
      comment_colour = merge_curline_colour( fd->attr+ATTR_CURLINE, fd->ecolour+ECOLOUR_COMMENTS );
   else
      comment_colour = set_colour( fd->ecolour+ECOLOUR_COMMENTS );

   for (; curr != NULL; curr = curr->next)
   {
      if ( !curr->line_comment )
         continue;
      len_comment = curr->len_start_delim;
      ptr = curr->start_delim;
      state = STATE_START;
      vcol = SCREEN_VIEW(scrno)->verify_col-1;
      switch( curr->column )
      {
         case 0:  /* 'any' column */
            for ( i = 0; i < len; i++ )
            {
               if ( fd->parser->case_sensitive )
               {
                  ch1 = work[i];
                  ch2 = *ptr;
               }
               else
               {
                  ch1 = tolower( work[i] );
                  ch2 = tolower( *ptr );
               }
               switch(state)
               {
                  case STATE_START:
                     if ( ch1 == ch2 )
                     {
                        state = STATE_COMMENT;
                        pos = i;
                        ptr++;
                        break;
                     }
                     break;
                  case STATE_COMMENT:
                     if ( ch2 == '\0')
                     {
                        state = STATE_MATCH;
                        break;
                     }
                     if ( ch1 != ch2 )
                     {
                        state = STATE_START;
                        ptr = curr->start_delim;
                        break;
                     }
                     ptr++;
                     break;
               }
               if ( state == STATE_MATCH )
                  break;
            }
            if ( state == STATE_MATCH
            || ( state == STATE_COMMENT && *ptr == '\0'))
            {
               TRACE_RETURN();
               return (long)pos;
            }
            break;
         case MAX_INT:  /* 'firstnonblank' */
            for ( i = 0; i < len; i++ )
            {
               if ( fd->parser->case_sensitive )
               {
                  ch1 = work[i];
                  ch2 = *ptr;
               }
               else
               {
                  ch1 = tolower( work[i] );
                  ch2 = tolower( *ptr );
               }
               switch(state)
               {
                  case STATE_START:
                     if ( ch1 != ' ' )
                     {
                        if ( ch1 == ch2 )
                        {
                           state = STATE_COMMENT;
                           pos = i;
                           ptr++;
                        }
                        else
                           state = STATE_IGNORE;
                     }
                     break;
                  case STATE_COMMENT:
                     if ( ch1 == ch2 )
                     {
                        ptr++;
                        break;
                     }
                     if ( ch2 == '\0' )
                     {
                        state = STATE_MATCH;
                        break;
                     }
                     state = STATE_IGNORE;
                     break;
               }
               if ( state == STATE_MATCH
               ||   state == STATE_IGNORE)
                  break;
            }
            if ( state == STATE_MATCH
            || ( state == STATE_COMMENT && *ptr == '\0'))
            {
               TRACE_RETURN();
               return (long)pos;
            }
            break;
         default:    /* specific column */
            for ( i = curr->column; i < len; i++ )
            {
               if ( fd->parser->case_sensitive )
               {
                  ch1 = work[i];
                  ch2 = *ptr;
               }
               else
               {
                  ch1 = tolower( work[i] );
                  ch2 = tolower( *ptr );
               }
               if ( ch2 == '\0' )
               {
                  TRACE_RETURN();
                  return (long)curr->column;
               }
               if ( ch2 != ch1 )
                  break;
               ptr++;
            }
            break;
      }
   }
   TRACE_RETURN();
   return -1L;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_single_comments(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_single_comments(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
   PARSE_COMMENTS *curr=fd->parser->first_comments;
   LENGTHTYPE i,vcol,pos=0,len=scurr->length,len_comment;
   CHARTYPE *ptr;
   int state;
   chtype comment_colour;
   CHARTYPE ch1,ch2;

   TRACE_FUNCTION( "parser.c:  parse_single_comments" );
   if ( scurr->is_current_line )
      comment_colour = merge_curline_colour( fd->attr+ATTR_CURLINE, fd->ecolour+ECOLOUR_COMMENTS );
   else
      comment_colour = set_colour( fd->ecolour+ECOLOUR_COMMENTS );

   for ( ; curr != NULL; curr = curr->next )
   {
      if ( !curr->line_comment )
         continue;
      len_comment = curr->len_start_delim;
      ptr = curr->start_delim;
      state = STATE_START;
      vcol = SCREEN_VIEW(scrno)->verify_col-1;
      switch( curr->column )
      {
         case 0:  /* 'any' column */
            for ( i = 0; i < len; i++ )
            {
               if ( fd->parser->case_sensitive )
               {
                  ch1 = work[i];
                  ch2 = *ptr;
               }
               else
               {
                  ch1 = tolower( work[i] );
                  ch2 = tolower( *ptr );
               }
               switch(state)
               {
                  case STATE_START:
                     if ( ch1 == ch2 )
                     {
                        state = STATE_COMMENT;
                        pos = i;
                        ptr++;
                        break;
                     }
                     break;
                  case STATE_COMMENT:
                     if ( ch2 == '\0' )
                     {
                        state = STATE_MATCH;
                        break;
                     }
                     if ( ch1 != ch2 )
                     {
                        state = STATE_START;
                        ptr = curr->start_delim;
                        break;
                     }
                     ptr++;
                     break;
               }
               if ( state == STATE_MATCH )
                  break;
            }
            if ( state == STATE_MATCH
            || ( state == STATE_COMMENT && *ptr == '\0' ) )
            {
               for ( i = pos; i < len; i++ )
               {
                  if ( i >= vcol && i-vcol < 260 )
                     scurr->highlighting[i-vcol] = comment_colour;
                  if ( work[i] != ' ' )
                  {
                     number_blanks++;
                     work[i] = ' ';
                  }
               }
            }
            break;
         case MAX_INT:  /* 'firstnonblank' */
            for ( i = 0; i < len; i++ )
            {
               if ( fd->parser->case_sensitive )
               {
                  ch1 = work[i];
                  ch2 = *ptr;
               }
               else
               {
                  ch1 = tolower( work[i] );
                  ch2 = tolower( *ptr );
               }
               switch( state )
               {
                  case STATE_START:
                     if ( ch1 != ' ' )
                     {
                        if ( ch1 == ch2 )
                        {
                           state = STATE_COMMENT;
                           pos = i;
                           ptr++;
                        }
                        else
                           state = STATE_IGNORE;
                     }
                     break;
                  case STATE_COMMENT:
                     if ( ch1 == ch2 )
                     {
                        ptr++;
                        break;
                     }
                     if ( ch2 == '\0' )
                     {
                        state = STATE_MATCH;
                        break;
                     }
                     state = STATE_IGNORE;
                     break;
               }
               if ( state == STATE_MATCH
               ||   state == STATE_IGNORE)
                  break;
            }
            if ( state == STATE_MATCH
            || ( state == STATE_COMMENT && *ptr == '\0' ) )
            {
               for ( i = pos; i < len; i++ )
               {
                  if ( i >= vcol && i-vcol < 260 )
                     scurr->highlighting[i-vcol] = comment_colour;
                  if ( work[i] != ' ' )
                  {
                     number_blanks++;
                     work[i] = ' ';
                  }
               }
            }
            break;
         default:    /* specific column */
            /*
             * Fixes for specific column parsing by Harald Vogt <hvogt@ifh.de>
             */
            for ( i = curr->column-1; i < len; i++ )
            {
               if ( fd->parser->case_sensitive )
               {
                  ch1 = work[i];
                  ch2 = *ptr;
               }
               else
               {
                  ch1 = tolower( work[i] );
                  ch2 = tolower( *ptr );
               }
               switch( state )
               {
                  case STATE_START:
                     if ( ch1 == ch2 )
                     {
                        state = STATE_COMMENT;
                        pos = i;
                        ptr++;
                     }
                     else
                        state = STATE_START;
                     break;
                  case STATE_COMMENT:
                     if ( ch2 == '\0' )
                     {
                        state = STATE_MATCH;
                        break;
                     }
                     if ( ch1 != ch2 )
                     {
                        state = STATE_START;
                        ptr = curr->start_delim;
                        break;
                     }
                     ptr++;
                     break;
               }
               if ( state == STATE_MATCH
               ||   state == STATE_START )
                  break;
            }
            if ( state == STATE_MATCH
            || ( state == STATE_COMMENT && *ptr == '\0' ) )
            {
               for ( i = pos; i < len; i++ )
               {
                  if ( i >= vcol && i-vcol < 260 )
                     scurr->highlighting[i-vcol] = comment_colour;
                  if (work[i] != ' ')
                  {
                     number_blanks++;
                     work[i] = ' ';
                  }
               }
            }
      }
   }
   TRACE_RETURN();
   return RC_OK;
}

/***********************************************************************/
#ifdef HAVE_PROTO
short parse_paired_comments(CHARTYPE scrno,FILE_DETAILS *fd)
#else
short parse_paired_comments(scrno,fd)
CHARTYPE scrno;
FILE_DETAILS *fd;
#endif
/***********************************************************************/
{
   PARSE_COMMENTS *curr_comments=fd->parser->first_comments;
   LENGTHTYPE i,j,k,vcol,len_start,len_end,len=0;
   long pos,comment_start_pos,start_line_comment=-1,comment_end_pos=-1;
   CHARTYPE *ptr,*ptr1;
   int type=0;
   int comment_level=0;
   chtype comment_colour,comment_colour1;
   SHOW_LINE *scurr,*scurr1;
   bool first=TRUE;

   TRACE_FUNCTION("parser.c:  parse_paired_comments");
#if 0
   /*
    * Find the line up to 100? lines above the first displayed line
    * and check for start/end
    * pairs of comment delimiters. At the end of this search, we
    * will be either in a state of STATE_START (ie not in a comment)
    * or STATE_COMMENT (ie inside a comment).
    * We also have the level of nesting at this point. For "nonest"
    * paired comments this can be a max of 1;
    */
   true_line = screen[scrno].sl->line_number;
   lineno = max(1,true_line-50);
   curr = lll_find(fd->first_line,fd->last_line,lineno,fd->number_lines);
   for (;lineno<true_line;lineno++)
   {
      pos = 0;
      while(1)
      {
         pos = find_comment(pos,curr->length+1,curr->line,curr_comments->start_delim,curr_comments->end_delim,&type);
         if (pos == -1)
            break;
         if (type == STATE_START_TAG)
         {
            comment_start_pos = 0;
            pos += len_start;
         }
         else
         {
            comment_start_pos = -1;
            pos += len_start;
         }
      }
   }
#endif

   work = brec;
   for(curr_comments=fd->parser->first_comments;(curr_comments!=NULL&&curr_comments->line_comment==TRUE);curr_comments=curr_comments->next);
   /*
    * comment_start_pos == -1 (not in the middle of a multi-line comment)
    * comment_start_pos != -1 (in the middle of a multi-line comment)
    */
   comment_start_pos = -1;
   comment_level = 0;
   /*
    * Now check each line in the display for paired comments
    */
   vcol = SCREEN_VIEW(scrno)->verify_col-1;
   len_start = strlen((DEFCHAR *)curr_comments->start_delim);
   len_end = strlen((DEFCHAR *)curr_comments->end_delim);
   for (i=0;i<screen[scrno].rows[WINDOW_FILEAREA];i++)
   {
      scurr = screen[scrno].sl + i;
      if (scurr->line_type == LINE_LINE
      &&  scurr->length > 0)
      {
         len = scurr->length;
         memcpy((DEFCHAR *)work,scurr->contents,len);
         work[len] = ' ';
         start_line_comment = find_single_comments( scrno, fd, scurr );
         ptr = work;
      }
      else
         ptr = NULL;
      if (scurr->is_current_line)
         comment_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_COMMENTS);
      else
         comment_colour = set_colour(fd->ecolour+ECOLOUR_COMMENTS);
      pos = 0;
      comment_end_pos = -1;
      while(ptr)
      {
         pos = find_comment( fd, pos, scurr->length+1, ptr, curr_comments->start_delim, curr_comments->end_delim, &type );
         if (pos == -1)
            break;
         /*
          * We have found a comment delimiter; start or end at the
          * returned pos.
          */
         if (type == STATE_START_TAG)
         {
            first = FALSE;
            comment_start_pos = pos;
            /*
             * If the first multi-line comment starts after a line comment
             * use the specs for the line comment to comment the line
             */
            if (start_line_comment != -1
            &&  start_line_comment < pos)
            {
               for (j=start_line_comment;j<len;j++)
               {
                  if (j >= vcol && j-vcol < 260)
                     scurr->highlighting[j-vcol] = comment_colour;
                     if (work[j] != ' ')
                     {
                        work[j] = ' ';
                     }
               }
               comment_start_pos = -1;
               type = 0;
               break;
            }
            pos += len_start;
         }
         else
         {
            /*
             * Got an end comment. If this is the first comment token
             * then we need to go back and comment all lines above this one.
             */
            if (first)
            {
               for (k=0;k<i;k++)
               {
                  scurr1 = screen[scrno].sl + k;
                  if (scurr1->is_current_line)
                     comment_colour1 = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_COMMENTS);
                  else
                     comment_colour1 = set_colour(fd->ecolour+ECOLOUR_COMMENTS);
                  if (scurr1->line_type == LINE_LINE)
                  {
                     ptr1 = scurr1->contents;
                     for (j=0;j<scurr1->length;j++)
                     {
                        if (j >= vcol && j-vcol < 260)
                           scurr1->highlighting[j-vcol] = comment_colour1;
                     }
                  }
               }
               comment_start_pos = 0;
            }
            first = FALSE;
            if (comment_start_pos != -1)
            {
               /*
                * We have closure of a comment pair.
                */
               for (j=comment_start_pos;j<pos+len_end;j++)
               {
                  if (j >= vcol && j-vcol < 260)
                     scurr->highlighting[j-vcol] = comment_colour;
                     if (work[j] != ' ')
                     {
                        work[j] = ' ';
                     }
               }
               comment_end_pos = pos;
               pos += len_end;
               comment_start_pos = -1;
            }
            else
              pos += len_end;
         }
      }
      if (ptr
      &&  type == STATE_START_TAG)
      {
         /*
          * The last delimiter was a start, so set the start pos for the
          * next line to 0, and comment out the remainder of the line.
          */
         for (j=comment_start_pos;j<scurr->length;j++)
         {
            if (j >= vcol && j-vcol < 260)
               scurr->highlighting[j-vcol] = comment_colour;
            if (work[j] != ' ')
            {
               work[j] = ' ';
            }
         }
         comment_start_pos = 0;
      }
      else
      {
         /*
          * If the last multi-line comment starts after a line comment
          * use the specs for the line comment to comment the line
          */
         if (ptr
         &&  start_line_comment != -1
         &&  start_line_comment > comment_end_pos
         &&  comment_start_pos == -1)
         {
            for (j=start_line_comment;j<len;j++)
            {
               if (j >= vcol && j-vcol < 260)
                  scurr->highlighting[j-vcol] = comment_colour;
                  if (work[j] != ' ')
                  {
                     work[j] = ' ';
                  }
            }
            comment_start_pos = -1;
            type = 0;
         }
      }
   }
   TRACE_RETURN();
   return RC_OK;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_strings(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_strings(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   int i;
   LENGTHTYPE first_quote,len=scurr->length;
   LENGTHTYPE vcol,off=0;
   chtype string_colour,incomplete_string_colour;
   CHARTYPE single_quote='\'';
   CHARTYPE double_quote='\"';
   CHARTYPE backslash='\\';
   CHARTYPE tab_character='\t';
   int state;
   bool check_single_quote=FALSE;
   bool check_double_quote=FALSE;
   bool backslash_single_quote=FALSE;
   bool backslash_double_quote=FALSE;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_strings");
   if (scurr->is_current_line)
   {
      string_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_STRINGS);
      incomplete_string_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_INC_STRING);
   }
   else
   {
      string_colour = set_colour(fd->ecolour+ECOLOUR_STRINGS);
      incomplete_string_colour = set_colour(fd->ecolour+ECOLOUR_INC_STRING);
   }
   vcol = SCREEN_VIEW(scrno)->verify_col-1;
   if (fd->parser->check_single_quote)
      check_single_quote = TRUE;
   if (fd->parser->check_double_quote)
      check_double_quote = TRUE;
   if (fd->parser->backslash_single_quote)
      backslash_single_quote = TRUE;
   if (fd->parser->backslash_double_quote)
      backslash_double_quote = TRUE;
   if (check_single_quote
   ||  check_double_quote)
   {
      first_quote = len+1;
      state = STATE_START;
      for (i=0;i<len;i++)
      {
         if (i >= vcol)
            off = i - vcol;
         switch(state)
         {
            case STATE_START:
               if (work[i] == single_quote
               &&  check_single_quote)
               {
                  state = STATE_SINGLE_QUOTE;
                  first_quote = i;
                  if (i >= vcol && off < 260)
                     scurr->highlighting[off] = string_colour;
                  if (work[i] != ' ')
                  {
                     number_blanks++;
                     work[i] = ' ';
                  }
                  break;
               }
               if (work[i] == double_quote
               &&  check_double_quote)
               {
                  state = STATE_DOUBLE_QUOTE;
                  first_quote = i;
                  if (i >= vcol && off < 260)
                     scurr->highlighting[off] = string_colour;
                  if (work[i] != ' ')
                  {
                     number_blanks++;
                     work[i] = ' ';
                  }
                  break;
               }
               /*
                * Convert all tab characters to spaces for later
                */
               if (work[i] == tab_character)
               {
                  number_blanks++;
                  work[i] = ' ';
                  break;
               }
               break;
            case STATE_SINGLE_QUOTE:
               if (work[i] == backslash)
               {
                  if (backslash_double_quote)
                  {
                     if (i+1 != len)
                     {
                        work[i+1] = ' ';
                        number_blanks++;
                     }
                  }
               }
               else if (work[i] == single_quote)
               {
                  state = STATE_START;
                  first_quote = len + 1;
               }
               if (i >= vcol && off < 260)
                  scurr->highlighting[off] = string_colour;
               if (work[i] != ' ')
               {
                  number_blanks++;
                  work[i] = ' ';
               }
               break;
            case STATE_DOUBLE_QUOTE:
               if (work[i] == backslash)
               {
                  if (backslash_double_quote)
                  {
                     if (i+1 != len)
                     {
                        work[i+1] = ' ';
                        number_blanks++;
                     }
                  }
               }
               else if (work[i] == double_quote)
               {
                  state = STATE_START;
                  first_quote = len + 1;
               }
               if (i >= vcol && off < 260)
                  scurr->highlighting[off] = string_colour;
               if (work[i] != ' ')
               {
                  number_blanks++;
                  work[i] = ' ';
               }
               break;
            default:
               break;
         }
      }
      if (first_quote < len)
      {
         for(i=first_quote;i<len;i++)
         {
            if (i >= vcol && i-vcol < 260)
               scurr->highlighting[i-vcol] = incomplete_string_colour;
         }
      }
   }
   TRACE_RETURN();
   return RC_OK;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_delimiters(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr,CHARTYPE *start,CHARTYPE *end,chtype colour)
#else
static short parse_delimiters(scrno,fd,scurr,start,end,colour)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
CHARTYPE *start,*end;
chtype colour;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   int i,j,state;
   LENGTHTYPE len=scurr->length,pos=0,len_plus_one;
   LENGTHTYPE vcol;
   CHARTYPE *ptr_start=start,*ptr_end=end;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_delimiters");
   vcol = SCREEN_VIEW(scrno)->verify_col-1;
   state = STATE_START;
   len_plus_one = len+1;
   for (i=0;i<len_plus_one;i++)
   {
      switch(state)
      {
         case STATE_START:
            if (work[i] == *ptr_start)
            {
               state = STATE_START_TAG;
               pos = i;
               ptr_start++;
               break;
            }
            break;
         case STATE_START_TAG:
            if (*ptr_start == '\0')
            {
               state = STATE_MATCH;
               ptr_start = start;
               ptr_end = end;
               break;
            }
            if (work[i] != *ptr_start)
            {
               state = STATE_START;
               ptr_start = start;
               break;
            }
            ptr_start++;
            break;
         case STATE_MATCH:  /* now look for end delim */
            if (work[i] == *ptr_end)
            {
               state = STATE_END_TAG;
               ptr_end++;
               break;
            }
            break;
         case STATE_END_TAG:
            if (*ptr_end == '\0')
            {
               /*
                * We have a match
                */
               for (j=pos;j<i;j++)
               {
                  if (work[j] != ' ')
                  {
                     if (j >= vcol && j-vcol < 260)
                        scurr->highlighting[j-vcol] = colour;
                     number_blanks++;
                     work[j] = ' ';
                  }
               }
               if (work[i] == *ptr_start)
               {
                  pos = i;
                  ptr_start++;
                  state = STATE_START_TAG;
               }
               else
                  state = STATE_START;
               break;
            }
            if (work[i] != *ptr_end)
            {
               state = STATE_START;
               ptr_start = start;
               break;
            }
            ptr_end++;
            break;
      }
   }
   TRACE_RETURN();
   return RC_OK;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_markup_tag(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_markup_tag(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   chtype colour;
   short rc;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_markup_tag");
   if (scurr->is_current_line)
      colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_HTML_TAG);
   else
      colour = set_colour(fd->ecolour+ECOLOUR_HTML_TAG);
   rc = parse_delimiters(scrno,fd,scurr,fd->parser->markup_tag_start_delim,fd->parser->markup_tag_end_delim,colour);
   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_markup_reference(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_markup_reference(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   chtype colour;
   short rc;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_markup_reference");
   if (scurr->is_current_line)
      colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_HTML_CHAR);
   else
      colour = set_colour(fd->ecolour+ECOLOUR_HTML_CHAR);
   rc = parse_delimiters(scrno,fd,scurr,fd->parser->markup_reference_start_delim,fd->parser->markup_reference_end_delim,colour);
   TRACE_RETURN();
   return rc;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_headers(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_headers(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_HEADERS *curr=fd->parser->first_header;
   LENGTHTYPE i,j,k,vcol,pos=0,len=scurr->length,len_header;
   CHARTYPE *ptr;
   int state;
   bool found=FALSE;
   chtype header_colour;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_headers");
   if (scurr->is_current_line)
      header_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_HEADER);
   else
      header_colour = set_colour(fd->ecolour+ECOLOUR_HEADER);

   vcol = SCREEN_VIEW(scrno)->verify_col-1;
   for (; curr != NULL; curr = curr->next)
   {
      len_header = curr->len_header_delim;
      ptr = curr->header_delim;
      state = STATE_START;
      switch(curr->header_column)
      {
         case 0:  /* 'any' column */
            for (i=0;i<len;i++)
            {
               /*
                * Ignore spaces, they are either "real" spaces or
                * already-highlighted characters
                */
               if ( work[i] == ' ' )
               {
                  i++;
                  continue;
               }
               switch(state)
               {
                  case STATE_START:
                     if (work[i] == *ptr)
                     {
                        state = STATE_HEADER;
                        pos = i;
                        ptr++;
                        break;
                     }
                     break;
                  case STATE_HEADER:
                     if (*ptr == '\0')
                     {
                        state = STATE_MATCH;
                        break;
                     }
                     if (work[i] != *ptr)
                     {
                        state = STATE_START;
                        ptr = curr->header_delim;
                        break;
                     }
                     ptr++;
                     break;
               }
               if (state == STATE_MATCH)
                  break;
            }
            if (state == STATE_MATCH
            || (state == STATE_HEADER && *ptr == '\0'))
            {
               for (i=pos;i<len;i++)
               {
                  if (i >= vcol && i-vcol < 260)
                     scurr->highlighting[i-vcol] = header_colour;
                  if (work[i] != ' ')
                  {
                     number_blanks++;
                     work[i] = ' ';
                  }
               }
               found = TRUE;
            }
            break;
         case MAX_INT:  /* 'firstnonblank' */
            for (i=0;i<len;i++)
            {
               switch(state)
               {
                  case STATE_START:
                     if (work[i] != ' ')
                     {
                        if (work[i] == *ptr)
                        {
                           state = STATE_HEADER;
                           pos = i;
                           ptr++;
                        }
                        else
                           state = STATE_IGNORE;
                     }
                     break;
                  case STATE_HEADER:
                     if (work[i] == *ptr)
                     {
                        ptr++;
                        break;
                     }
                     if (*ptr == '\0')
                     {
                        state = STATE_MATCH;
                        break;
                     }
                     state = STATE_IGNORE;
                     break;
               }
               if (state == STATE_MATCH
               ||  state == STATE_IGNORE)
                  break;
            }
            if (state == STATE_MATCH
            || (state == STATE_HEADER && *ptr == '\0'))
            {
               for (i=pos;i<len;i++)
               {
                  if (i >= vcol && i-vcol < 260)
                     scurr->highlighting[i-vcol] = header_colour;
                  if (work[i] != ' ')
                  {
                     number_blanks++;
                     work[i] = ' ';
                  }
               }
               found = TRUE;
            }
            break;
         default:    /* specific column */
            for (i=curr->header_column-1,j=0;i<len;i++,j++,ptr++)
            {
               if (*ptr == '\0')
               {
                  for (k=curr->header_column-1;k<len;k++)
                  {
                     if (k >= vcol && k-vcol < 260)
                        scurr->highlighting[k-vcol] = header_colour;
                     if (work[k] != ' ')
                     {
                        number_blanks++;
                        work[k] = ' ';
                     }
                  }
                  found = TRUE;
                  break;
               }
               if (*ptr != work[i])
                  break;
            }
            break;
      }
      if (found)
         break;
   }
   TRACE_RETURN();
   return RC_OK;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_labels(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_labels(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
   LENGTHTYPE vcol,pos=0,end=0,len=scurr->length,len_label;
   long i,j,k;
   CHARTYPE *ptr;
   int state;
   chtype label_colour;

   TRACE_FUNCTION("parser.c:  parse_labels");
   if ( scurr->is_current_line )
      label_colour = merge_curline_colour( fd->attr+ATTR_CURLINE, fd->ecolour+ECOLOUR_LABEL );
   else
      label_colour = set_colour( fd->ecolour+ECOLOUR_LABEL );

   len_label = fd->parser->len_label_delim;
   ptr = fd->parser->label_delim;
   vcol = SCREEN_VIEW(scrno)->verify_col-1;
   if ( len_label == 0 )
   {
      /*
       * column n
       */
      pos = fd->parser->label_column-1;
      if ( len > pos
      &&   work[pos] != ' ' )
      {
         for (i=pos;i<len;i++)
         {
            if (i >= vcol && i-vcol < 260)
               scurr->highlighting[i-vcol] = label_colour;
            if (work[i] != ' ')
            {
               number_blanks++;
               work[i] = ' ';
            }
         }
      }
   }
   else
   {
      /*
       * delimiter ...
       */
      state = STATE_START;
      switch( fd->parser->label_column )
      {
         case 0:  /* 'any' column */
            for ( i = 0; i < len; i++ )
            {
               /*
                * Ignore spaces, they are either "real" spaces or
                * already-highlighted characters
                */
               if ( work[i] == ' ' )
               {
                  continue;
               }
               switch(state)
               {
                  case STATE_START:
                     if (work[i] == *ptr)
                     {
                        state = STATE_LABEL;
                        pos = i;
                        ptr++;
                        break;
                     }
                     break;
                  case STATE_LABEL:
                     if ( *ptr == '\0' )
                     {
                        state = STATE_MATCH;
                        break;
                     }
                     if (work[i] != *ptr)
                     {
                        state = STATE_START;
                        ptr = fd->parser->label_delim;
                        break;
                     }
                     ptr++;
                     break;
               }
               if (state == STATE_MATCH)
                  break;
            }
            if (state == STATE_MATCH
            || (state == STATE_LABEL && *ptr == '\0'))
            {
               for (i=pos;i>-1;i--)
               {
                  if (i >= vcol && i-vcol < 260)
                     scurr->highlighting[i-vcol] = label_colour;
                  if (work[i] != ' ')
                  {
                     number_blanks++;
                     work[i] = ' ';
                  }
               }
            }
            break;
         /*
          * firstnonblank
          * the first word on the line must end with the specified
          * delimiter. eg.
          *     label::  
          *     -------  is the valid label if '::' is the delimiter
          *     junk label:: 
          *              is not
          *     junk label::
          *              is not
          */
         case MAX_INT:  /* 'firstnonblank' */
            end = len;
            for ( i = 0; i < len; i++ )
            {
               switch( state )
               {
                  case STATE_START:
                     if (work[i] != ' ')
                     {
                        state = STATE_LABEL;
                        pos = i;
                     }
                     break;
                  case STATE_LABEL:
                     if ( work[i] == ' ' )
                     {
                        state = STATE_IGNORE;
                     }
                     else if ( work[i] == *ptr )
                     {
                        state = STATE_DELIMITER;
                        ptr++;
                     }
                     break;
                  case STATE_DELIMITER:
                     if ( *ptr == '\0' )
                     {
                        end = i;
                        state = STATE_MATCH;
                        break;
                     }
                     if ( work[i] == *ptr )
                     {
                        ptr++;
                        break;
                     }
                     state = STATE_IGNORE;
                     break;
               }
               if ( state == STATE_MATCH
               ||   state == STATE_IGNORE)
                  break;
            }
            if ( state == STATE_MATCH
            || ( state == STATE_DELIMITER && *ptr == '\0' ) )
            {
               for ( i = pos; i < end; i++)
               {
                  if ( i >= vcol && i-vcol < 260 )
                     scurr->highlighting[i-vcol] = label_colour;
                  if ( work[i] != ' ' )
                  {
                     number_blanks++;
                     work[i] = ' ';
                  }
               }
            }
            break;
         default:    /* specific column */
            for (i=fd->parser->label_column-1,j=0;i<len;i++,j++,ptr++)
            {
               if (*ptr == '\0')
               {
                  for (k=fd->parser->label_column-1;k>-1;k--)
                  {
                     if (k >= vcol && k-vcol < 260)
                        scurr->highlighting[k-vcol] = label_colour;
                     if (work[k] != ' ')
                     {
                        number_blanks++;
                        work[k] = ' ';
                     }
                  }
                  break;
               }
               if (*ptr != work[i])
                  break;
            }
            break;
      }
   }
   TRACE_RETURN();
   return RC_OK;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_match(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_match(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   int i;
   LENGTHTYPE len=scurr->length;
   LENGTHTYPE vcol,off=0;
   chtype paren_colour;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_match");
   /*
    * This is incomplete. It needs to determine the level of bracket
    * matching and to check multiple lines.
    */
   if (scurr->is_current_line)
      paren_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_LEVEL_1_PAREN);
   else
      paren_colour = set_colour(fd->ecolour+ECOLOUR_LEVEL_1_PAREN);

   vcol = SCREEN_VIEW(scrno)->verify_col-1;
   for (i=0;i<len;i++)
   {
      if (i >= vcol)
         off = i - vcol;
      if (work[i] == '('
      ||  work[i] == ')'
      ||  work[i] == '['
      ||  work[i] == ']'
      ||  work[i] == '{'
      ||  work[i] == '}')
      {
         if (i >= vcol && off < 260)
            scurr->highlighting[off] = paren_colour;
         work[off] = ' ';
         number_blanks++;
      }
   }
   TRACE_RETURN();
   return RC_OK;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_postcompare(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_postcompare(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_POSTCOMPARE *curr;
   LENGTHTYPE i,j,vcol,len=scurr->length;
   bool found_for_column,individual_found;
   chtype postcompare_colour, original_postcompare_colour;
   long re_len;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_postcompare");
   if (scurr->is_current_line)
      original_postcompare_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_KEYWORDS);
   else
      original_postcompare_colour = set_colour(fd->ecolour+ECOLOUR_KEYWORDS);

   vcol = SCREEN_VIEW(scrno)->verify_col-1;

   for (i=0;i<len;)
   {
      /*
       * For the most common non-relevant character; ' ' check it
       * manually, because its quicker than re_match().
       */
      if (work[i] == ' ')
      {
         i++;
         continue;
      }
      found_for_column = FALSE;
      for ( curr = fd->parser->first_postcompare; curr != NULL; curr = curr->next)
      {
         individual_found = FALSE;
         switch(curr->is_class_type)
         {
            case TRUE:  /* CLASS type */
               re_len = re_match(&curr->pattern_buffer,(DEFCHAR *)work+i,len-i,0,0);
               if ( re_len > 0 )
               {
                  individual_found = TRUE;
               }
               break;
            default:  /* STRING type */
               re_len = strlen((DEFCHAR*)curr->string);
               if ( re_len <= len-i
               &&   memcmp((DEFCHAR*)curr->string,(DEFCHAR*)work+i, re_len) == 0 )
               {
                  individual_found = TRUE;
               }
               break;
         }
         if ( individual_found )
         {
            if (curr->alternate != 255) /* was an alternate colour specified for this postcompare */
            {
               if (scurr->is_current_line)
                  postcompare_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+curr->alternate);
               else
                  postcompare_colour = set_colour(fd->ecolour+curr->alternate);
            }
            else
              postcompare_colour = original_postcompare_colour;
            for (j=0;j<re_len;j++,i++)
            {
               if ( work[i] != ' ' )
               {
                  if (i >= vcol && i-vcol < 260)
                     scurr->highlighting[i-vcol] = postcompare_colour;
                  work[i] = ' ';
                  number_blanks++;
               }
            }
            found_for_column = TRUE;
         }
      }
      if ( !found_for_column )
         i++;
   }

   TRACE_RETURN();
   return RC_OK;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_columns(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_columns(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   LENGTHTYPE i,j,vcol,len=scurr->length;
   chtype column_colour, original_column_colour;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_columns");
   if (scurr->is_current_line)
      original_column_colour = set_colour(fd->attr+ATTR_CURLINE);
   else
      original_column_colour = set_colour(fd->attr+ATTR_FILEAREA);

   vcol = SCREEN_VIEW(scrno)->verify_col-1;


   for ( i=0; i<fd->parser->number_columns; i++)
   {
      if (fd->parser->column_alternate[i] != 255) /* was an alternate colour specified for this column */
      {
         if (scurr->is_current_line)
            column_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+fd->parser->column_alternate[i]);
         else
            column_colour = set_colour(fd->ecolour+fd->parser->column_alternate[i]);
      }
      else
         column_colour = original_column_colour;
      for (j=fd->parser->first_column[i]-1; j<len && j<fd->parser->last_column[i]-1; j++)
      {
         if (j >= vcol && j-vcol < 260)
            scurr->highlighting[j-vcol] = column_colour;
         work[j] = ' ';
         number_blanks++;
       }
   }

   TRACE_RETURN();
   return RC_OK;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
short find_preprocessor(FILE_DETAILS *fd, CHARTYPE *word, int len, int *alternate_colour)
#else
short find_preprocessor(fd, word, len,alternate_colour)
FILE_DETAILS *fd;
CHARTYPE *word; 
int len,*alternate_colour;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_KEYWORDS *curr=fd->parser->first_keyword;
   CHARTYPE preprocessor_char=fd->parser->preprocessor_char;
   bool found;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  find_preprocessor");
   for (; curr != NULL; curr = curr->next)
   {
      if (len == curr->keyword_length-1)
      {
         found = FALSE;
         if (*(curr->keyword) == preprocessor_char)
         {
            if (fd->parser->case_sensitive)
            {
               if (memcmp(word,curr->keyword+1,len) == 0)
                  found = TRUE;
            }
            else
            {
               if (memcmpi(word,curr->keyword+1,len) == 0)
                  found = TRUE;
            }
         }
         if (found)
         {
            *alternate_colour = curr->alternate;
            TRACE_RETURN();
            return len;
         }
      }
   }
   TRACE_RETURN();
   return 0;
}
/***********************************************************************/
#ifdef HAVE_PROTO
bool find_keyword(FILE_DETAILS *fd, CHARTYPE *word, int len,int *alternate_colour)
#else
bool find_keyword(fd, word, len, alternate_colour)
FILE_DETAILS *fd;
CHARTYPE *word; 
int len;
int *alternate_colour;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_KEYWORDS *curr=fd->parser->first_keyword;
   bool found=FALSE;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  find_keyword");
   for (; curr != NULL; curr = curr->next)
   {
      if (len == curr->keyword_length)
      {
         if (fd->parser->case_sensitive)
         {
            if (memcmp(word,curr->keyword,len) == 0)
               found = TRUE;
         }
         else
         {
            if (memcmpi(word,curr->keyword,len) == 0)
               found = TRUE;
         }
         if (found)
         {
            *alternate_colour = curr->alternate;
            break;
         }
      }
   }
   TRACE_RETURN();
   return found;
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_keywords(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_keywords(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
   int i,j,alt;
   LENGTHTYPE len=scurr->length;
   LENGTHTYPE vcol;
   long re_len, save_re_len;
   chtype keyword_colour, original_keyword_colour;
   chtype number_colour;


   TRACE_FUNCTION("parser.c:  parse_keywords");
   if ( scurr->is_current_line )
   {
      original_keyword_colour = merge_curline_colour( fd->attr+ATTR_CURLINE, fd->ecolour+ECOLOUR_KEYWORDS );
      number_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_NUMBERS);
   }
   else
   {
      original_keyword_colour = set_colour( fd->ecolour+ECOLOUR_KEYWORDS );
      number_colour = set_colour(fd->ecolour+ECOLOUR_NUMBERS);
   }

   vcol = SCREEN_VIEW(scrno)->verify_col-1;
   for ( i = 0 ; i < len; )
   {
      /*
       * For the most common non-relevant character; ' ' check it
       * manually, because its quicker than re_match().
       */
      if (work[i] == ' ')
      {
         i++;
      }
      else
      {
         /*
          * Find an identfier
          */
         re_len = re_match(&fd->parser->body_pattern_buffer, (DEFCHAR *)work+i, len-i, 0, 0 );
         if (re_len < 0)
         {
            /*
             * No identifier, is it a number ??
             */
            if ( fd->parser->have_number_pattern_buffer
            &&  CURRENT_VIEW->syntax_headers & HEADER_NUMBER )
            {
               re_len = re_match( &fd->parser->number_pattern_buffer, (DEFCHAR *)work+i, len-i, 0, 0 );
               if ( re_len > 0 )
               {
                  for ( j = 0 ; j < re_len; j++, i++ )
                  {
                     if ( i >= vcol && i-vcol < 260 )
                        scurr->highlighting[i-vcol] = number_colour;
                     if ( work[i] != ' ' )
                     {
                        work[i] = ' ';
                        number_blanks++;
                     }
                  }
               }
               else
               {
                  i++;
               }
            }
            else
            {
               i++;
            }
         }
         else
         {
            /*
             * We now have a match of an identifier; see if its a keyword
             */
            if ( work[i+re_len-1] == ' ' ) /* bug in regex()! */
               re_len--;
            save_re_len = re_len;
            if ( find_keyword( fd, work + i, re_len, &alt ) )
            {
               /*
                * Have found a matching keyword...
                */
               if ( alt != 255 ) /* was an alternate colour specified for this keyword */
               {
                  if (scurr->is_current_line)
                     keyword_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+alt);
                  else
                     keyword_colour = set_colour(fd->ecolour+alt);
               }
               else
                  keyword_colour = original_keyword_colour;
               for ( j = 0; j < re_len; j++, i++ )
               {
                  if (i >= vcol && i-vcol < 260)
                     scurr->highlighting[i-vcol] = keyword_colour;
                  work[i] = ' ';
                  number_blanks++;
               }
            }
            else
            {
               /*
                * We have NOT found a keyword, so check if its a number.
                * If not a number, simply remove the characters
                * from the line so it can no longer match any further
                * token types.
                */
               if ( fd->parser->have_number_pattern_buffer
               &&  CURRENT_VIEW->syntax_headers & HEADER_NUMBER )
               {
                  re_len = re_match( &fd->parser->number_pattern_buffer, (DEFCHAR *)work+i, len-i, 0, 0 );
                  if ( re_len > 0 )
                  {
                     for ( j = 0 ; j < re_len; j++, i++ )
                     {
                        if ( i >= vcol && i-vcol < 260 )
                           scurr->highlighting[i-vcol] = number_colour;
                        if ( work[i] != ' ' )
                        {
                           work[i] = ' ';
                           number_blanks++;
                        }
                     }
                  }
                  else
                  {
                     for ( j = 0; j < save_re_len; j++, i++ )
                     {
                        if ( work[i] != ' ' )
                        {
                           work[i] = ' ';
                           number_blanks++;
                        }
                     }
                  }
               }
               else
               {
                  for ( j = 0; j < save_re_len; j++, i++ )
                  {
                     if ( work[i] != ' ' )
                     {
                        work[i] = ' ';
                        number_blanks++;
                     }
                  }
               }
            }
         }
      }
   }
   TRACE_RETURN();
   return RC_OK;
}

/***********************************************************************/
#ifdef HAVE_PROTO
bool find_function(FILE_DETAILS *fd, CHARTYPE *word, int len,int *alternate_colour)
#else
bool find_function(fd, word, len, alternate_colour)
FILE_DETAILS *fd;
CHARTYPE *word; 
int len;
int *alternate_colour;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSE_FUNCTIONS *curr=fd->parser->first_function;
   bool found=FALSE;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  find_function");
   for (; curr != NULL; curr = curr->next)
   {
      if (len == curr->function_length)
      {
         if (fd->parser->case_sensitive)
         {
            if (memcmp(word,curr->function,len) == 0)
               found = TRUE;
         }
         else
         {
            if (memcmpi(word,curr->function,len) == 0)
               found = TRUE;
         }
         if (found)
         {
            *alternate_colour = curr->alternate;
            break;
         }
      }
   }
   TRACE_RETURN();
   return found;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_functions(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_functions(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
   int i,j,alt;
   LENGTHTYPE len=scurr->length;
   LENGTHTYPE vcol,function_len;
   long re_len;
   chtype function_colour, original_function_colour;
   CHARTYPE *word=NULL;
   int k,word_start,word_end,state;

   TRACE_FUNCTION( "parser.c:  parse_functions" );
   if ( scurr->is_current_line )
      original_function_colour = merge_curline_colour( fd->attr+ATTR_CURLINE, fd->ecolour+ECOLOUR_FUNCTIONS );
   else
      original_function_colour = set_colour( fd->ecolour+ECOLOUR_FUNCTIONS );

   vcol = SCREEN_VIEW(scrno)->verify_col - 1;
   for ( i = 0; i < len; )
   {
      /*
       * For the most common non-relevant character; ' ' check it
       * manually, because it should be quicker than re_match().
       */
      if (work[i] == ' ')
      {
         i++;
      }
      else
      {
         /*
          * We have an indentfier
          */
         re_len = re_match( &fd->parser->function_pattern_buffer, (DEFCHAR *)work + i, len - i, 0, 0 );
         if ( re_len < 0 )
         {
            /*
             * If we have the option REXX setting, then the identifier
             * we have could be a function (without the brackets) eg
             * Call left
             * Use the pattern buffer for identifiers to get to locate
             * the possible function.
             */
            if ( fd->parser->rexx_option )
            {
               re_len = re_match( &fd->parser->body_pattern_buffer, (DEFCHAR *)work + i, len - i, 0, 0 );
               if ( re_len < 0 )
               {
                  /*
                   * No identifier found, try the next char...
                   */
                  i++;
                  continue;
               }
               else
               {
                  /*
                   * Need to check that the previous word is "call".
                   * Then we can say we have a potential Rexx function
                   */
                  for ( word_start = 0, word_end = 0, state= STATE_START, k = i - 1; k > 0; k-- )
                  {
                      switch( state )
                      {
                         case STATE_START:
                            if ( work[k] != ' ')
                            {
                               state = STATE_START_TAG;
                               word_end = k;
                            }
                            break;
                         case STATE_START_TAG:
                            if ( work[k] == ' ')
                            {
                               state = STATE_END_TAG;
                               word_start = k+1;
                            }
                            break;
                      }
                      if ( state == STATE_END_TAG )
                         break;
                  }
                  if ( word_end-word_start == 3
                  &&   memcmpi( work+word_start, (CHARTYPE *)"call", 4 ) == 0
                  &&   word_end+1 != i )
                  {
                     /*
                      * We have found the CALL keyword. Process it here so
                      * that subsequent words on this line do not get set
                      * incorrectly.
                      */
                     chtype keyword_colour, original_keyword_colour;
                     if ( scurr->is_current_line )
                        original_keyword_colour = merge_curline_colour( fd->attr+ATTR_CURLINE, fd->ecolour+ECOLOUR_KEYWORDS );
                     else
                        original_keyword_colour = set_colour( fd->ecolour+ECOLOUR_KEYWORDS );
                     if ( find_keyword( fd, (CHARTYPE *)"call", 4, &alt ) )
                     {
                        if ( alt != 255 ) /* was an alternate colour specified for this keyword */
                        {
                           if ( scurr->is_current_line )
                              keyword_colour = merge_curline_colour( fd->attr+ATTR_CURLINE, fd->ecolour+alt );
                           else
                              keyword_colour = set_colour( fd->ecolour+alt );
                        }
                        else
                           keyword_colour = original_keyword_colour;
                        for ( j = 0; j < 4; j++ )
                        {
                           if ( word_start+j >= vcol && (word_start+j)-vcol < 260 )
                              scurr->highlighting[(word_start+j)-vcol] = keyword_colour;
                           if ( work[word_start+j] != ' ' )
                           {
                              work[word_start+j] = ' ';
                              number_blanks++;
                           }
                        }
                     }
                  }
                  else
                  {
                     i++;
                     continue;
                  }
               }
            }
            else
            {
               /*
                * Not REXX option, so no function found, try the next char...
                */
               i++;
               continue;
            }
         }
         /*
          * We now have a match of a function. Find the matchable
          * string by locating either the function_char or
          * the first space.
          */
         word = work + i;
         for ( function_len = 0; function_len < re_len; function_len++, word++ )
         {
            if ( *word == ' '
            ||   *word == fd->parser->function_char )
            {
               /*
                * We MUST always get here!
                */
               break;
            }
         }
         if ( find_function( fd, work + i, function_len, &alt ) )
         {
            if ( alt != 255 ) /* was an alternate colour specified for this keyword */
            {
               if ( scurr->is_current_line )
                  function_colour = merge_curline_colour( fd->attr+ATTR_CURLINE, fd->ecolour+alt );
               else
                  function_colour = set_colour( fd->ecolour+alt );
            }
            else
               function_colour = original_function_colour;
            for ( j = 0; j < function_len; j++, i++ )
            {
               if ( i >= vcol && i-vcol < 260 )
                  scurr->highlighting[i-vcol] = function_colour;
               work[i] = ' ';
               number_blanks++;
            }
         }
         else
         {
            /*
             * We haven't found an explicit function, so check if we have the
             * wildcard option set with an alternate colour
             */
            if ( fd->parser->have_function_option_alternate )
            {
               /*
                * OK, we have the option, but we must exclude explicit keywords
                * as a construct like in C: 
                *   if ( expr )
                * would be treated as a function call.
                */
               if ( !find_keyword( fd, work + i, function_len, &alt ) )
               {
                  alt = fd->parser->function_option_alternate;
                  if ( scurr->is_current_line )
                     function_colour = merge_curline_colour( fd->attr+ATTR_CURLINE, fd->ecolour+alt );
                  else
                     function_colour = set_colour( fd->ecolour+alt );
                  for ( j = 0; j < function_len; j++, i++)
                  {
                     if ( i >= vcol && i-vcol < 260 )
                        scurr->highlighting[i-vcol] = function_colour;
                     if ( word[i] != ' ' )
                     {
                        work[i] = ' ';
                        number_blanks++;
                     }
                  }
               }
               else
               {
                  i+= re_len;
               }
            }
            else
            {
               i+= re_len;
            }
         }
      }
   }
   TRACE_RETURN();
   return RC_OK;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_preprocessor(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr)
#else
static short parse_preprocessor(scrno,fd,scurr)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   int i,j,k,len_plus_one,state,alt;
   LENGTHTYPE len=scurr->length,preprocessor_char_start_pos=0;
   LENGTHTYPE vcol,keyword_length,keyword_start_pos=0,re_len;
   chtype keyword_colour,original_keyword_colour;
   CHARTYPE *word=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_preprocessor");
   if (scurr->is_current_line)
      original_keyword_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+ECOLOUR_PREDIR);
   else
      original_keyword_colour = set_colour(fd->ecolour+ECOLOUR_PREDIR);

   state = STATE_START;
   vcol = SCREEN_VIEW(scrno)->verify_col-1;
   len_plus_one = len + 1;
   for (i=0;i<len_plus_one;i++)
   {
      switch(state)
      {
         case STATE_START:
            if (work[i] == ' ')
               break;
            if (work[i] == fd->parser->preprocessor_char)
            {
               preprocessor_char_start_pos = i;
               state = STATE_OPTION;
               break;
            }
            state = STATE_IGNORE;
            break;
         case STATE_OPTION:
            if (work[i] == ' ')
               break;
            word = work+i;
            keyword_start_pos = i;
            state = STATE_STRING;
            break;
         case STATE_STRING:
            if (work[i] == ' ')
            {
               keyword_length = i - keyword_start_pos;
               if (keyword_length < fd->parser->min_keyword_length)
               {
                  state = STATE_START;
                  break;
               }
               work[i] = '\0';
               if ((re_len = find_preprocessor(fd,word,keyword_length,&alt)))
               {
                  if (alt != 255) /* was an alternate colour specified for this keyword */
                  {
                     if (scurr->is_current_line)
                        keyword_colour = merge_curline_colour(fd->attr+ATTR_CURLINE,fd->ecolour+alt);
                     else
                        keyword_colour = set_colour(fd->ecolour+alt);
                  }
                  else
                     keyword_colour = original_keyword_colour;
                  for (j=0,k=keyword_start_pos;j<re_len;j++,k++)
                  {
                     if (k >= vcol && k-vcol < 260)
                        scurr->highlighting[k-vcol] = keyword_colour;
                     work[k] = ' ';
                     number_blanks++;
                  }
                  if (preprocessor_char_start_pos >= vcol && preprocessor_char_start_pos-vcol < 260)
                     scurr->highlighting[preprocessor_char_start_pos-vcol] = keyword_colour;
                  work[preprocessor_char_start_pos] = ' ';
                  number_blanks++;
               }
               /* 
                * Don't increment number_blanks; it
                * was a blank before we set it to nul
                */
               work[i] = ' '; 
               state = STATE_IGNORE;
            }
         case STATE_IGNORE:
            break;
      }
      if (state == STATE_IGNORE)
      {
         break;
      }
   }
   TRACE_RETURN();
   return RC_OK;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
short parse_line(CHARTYPE scrno,FILE_DETAILS *fd,SHOW_LINE *scurr,short start_row)
#else
short parse_line(scrno,fd,scurr,start_row)
CHARTYPE scrno;
FILE_DETAILS *fd;
SHOW_LINE *scurr;
short start_row;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   int i;
   LENGTHTYPE len=scurr->length;
   chtype normal_colour;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_line");
   if (len == 0)
   {
      TRACE_RETURN();
      return RC_OK;
   }
   memcpy((DEFCHAR *)brec,scurr->contents,len);
   brec[len] = ' ';
   work = brec;

   /*
    * Set all columns to default colour.
    */
   if (scurr->is_current_line)
      normal_colour = set_colour(fd->attr+ATTR_CURLINE);
   else
      normal_colour = set_colour(fd->attr+ATTR_FILEAREA);
   for (i=0;i<260;i++)
   {
      scurr->highlighting[i] = normal_colour;
   }
   number_blanks = 0;
   /*
    * Parse columns - must be done first
    */
   if (fd->parser->have_columns
   &&  CURRENT_VIEW->syntax_headers & HEADER_COLUMN )
   {
      parse_columns(scrno,fd,scurr);
      if (number_blanks == len)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Parse single and or double quoted strings
    * No multi-line strings.
    */
   if (fd->parser->have_string
   &&  CURRENT_VIEW->syntax_headers & HEADER_STRING )
   {
      parse_strings(scrno,fd,scurr);
      if (number_blanks == len)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Parse headers
    */
   if (fd->parser->have_headers
   &&  CURRENT_VIEW->syntax_headers & HEADER_HEADER )
   {
      parse_headers(scrno,fd,scurr);
      if (number_blanks == len)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Parse single line comments , if we don't have any paired comments
    */
   if (!fd->parser->have_paired_comments
   &&  CURRENT_VIEW->syntax_headers & HEADER_COMMENT )
   {
      parse_single_comments(scrno,fd,scurr);
      if (number_blanks == len)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Parse DELIMITER labels first (COLUMN labels done later)
    */
   if ( fd->parser->len_label_delim
   &&  CURRENT_VIEW->syntax_headers & HEADER_LABEL )
   {
      parse_labels(scrno,fd,scurr);
      if (number_blanks == len)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Find functions; if applicable
    */
   if (fd->parser->function_option
   &&  CURRENT_VIEW->syntax_headers & HEADER_FUNCTION )
   {
      parse_functions(scrno,fd,scurr);
      if (number_blanks == len+1)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Parse markup references if applicable
    */
   if (fd->parser->have_markup_reference
   &&  CURRENT_VIEW->syntax_headers & HEADER_MARKUP )
   {
      parse_markup_reference(scrno,fd,scurr);
      if (number_blanks == len+1)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Parse markup tags if applicable
    */
   if ( fd->parser->have_markup_tag
   &&   CURRENT_VIEW->syntax_headers & HEADER_MARKUP )
   {
      parse_markup_tag(scrno,fd,scurr);
      if (number_blanks == len+1)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Parse parens/braces/brackets
    */
   if ( fd->parser->have_match
   &&   CURRENT_VIEW->syntax_headers & HEADER_MATCH )
   {
      parse_match( scrno, fd, scurr );
      if ( number_blanks == len )
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Find preprocesor keywords; if applicable
    */
   if (fd->parser->preprocessor_option)
   {
      parse_preprocessor(scrno,fd,scurr);
      if (number_blanks == len+1)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Find keywords, numbers, and remove identifiers
    */
   if (fd->parser->have_body_pattern_buffer
   &&  CURRENT_VIEW->syntax_headers & HEADER_KEYWORD )
   {
      parse_keywords(scrno,fd,scurr);
      if (number_blanks == len+1)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Parse COLUMN labels (DELIMITER labels done earlier)
    */
   if ( fd->parser->len_label_delim == 0
   &&  CURRENT_VIEW->syntax_headers & HEADER_LABEL )
   {
      parse_labels(scrno,fd,scurr);
      if (number_blanks == len)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   /*
    * Parse postcompare - always last
    */
   if (fd->parser->have_postcompare
   &&  CURRENT_VIEW->syntax_headers & HEADER_POSTCOMPARE )
   {
      parse_postcompare(scrno,fd,scurr);
      if (number_blanks == len+1)
      {
         TRACE_RETURN();
         return RC_OK;
      }
   }
   TRACE_RETURN();
   return RC_OK;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_case(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_case(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_case");
   if (memcmpi(line,(CHARTYPE *)"ignore",line_length) == 0)
      parser->case_sensitive = FALSE;
   else if (memcmpi(line,(CHARTYPE *)"respect",line_length) == 0)
      parser->case_sensitive = TRUE;
   else
   {
      sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",line,lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      rc = RC_INVALID_OPERAND;
   }
   TRACE_RETURN();
   return rc;
}
  
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_markup(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_markup(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CONMAR_PARAMS  3
   short rc=RC_OK;
   CHARTYPE *word[CONMAR_PARAMS+1];
   CHARTYPE strip[CONMAR_PARAMS];
   unsigned short num_params=0;
   short option=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_markup");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   num_params = param_split(line,word,CONMAR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params != 3)
   {
      sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if (equal((CHARTYPE *)"tag",word[0],3))
      option = 1;
   else if (equal((CHARTYPE *)"reference",word[0],9))
      option = 2;
   else
   {
      sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[0],lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if (strlen((DEFCHAR *)word[1]) > MAX_DELIMITER_LENGTH)
   {
      sprintf((DEFCHAR *)tmp,"Token too long '%s', line %d. Length should be <= %d",(DEFCHAR *)word[1],lineno,MAX_DELIMITER_LENGTH);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if (strlen((DEFCHAR *)word[2]) > MAX_DELIMITER_LENGTH)
   {
      sprintf((DEFCHAR *)tmp,"Token too long '%s', line %d. Length should be <= %d",(DEFCHAR *)word[2],lineno,MAX_DELIMITER_LENGTH);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if (option == 1)
   {
      parser->have_markup_tag = TRUE;
      strcpy((DEFCHAR *)parser->markup_tag_start_delim,(DEFCHAR *)word[1]);
      parser->len_markup_tag_start_delim = strlen((DEFCHAR *)word[1]);
      strcpy((DEFCHAR *)parser->markup_tag_end_delim,(DEFCHAR *)word[2]);
      parser->len_markup_tag_end_delim = strlen((DEFCHAR *)word[2]);
   }
   else
   {
      parser->have_markup_reference = TRUE;
      strcpy((DEFCHAR *)parser->markup_reference_start_delim,(DEFCHAR *)word[1]);
      parser->len_markup_reference_start_delim = strlen((DEFCHAR *)word[1]);
      strcpy((DEFCHAR *)parser->markup_reference_end_delim,(DEFCHAR *)word[2]);
      parser->len_markup_reference_end_delim = strlen((DEFCHAR *)word[2]);
   }
   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_string(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_string(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CONSTR_PARAMS  3
   short rc=RC_OK;
   CHARTYPE *word[CONSTR_PARAMS+1];
   CHARTYPE strip[CONSTR_PARAMS];
   unsigned short num_params=0;
   short option=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_string");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   num_params = param_split(line,word,CONSTR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);

   if (equal((CHARTYPE *)"single",word[0],6))
      option = STATE_SINGLE_QUOTE;
   else if (equal((CHARTYPE *)"double",word[0],6))
      option = STATE_DOUBLE_QUOTE;
   else if (equal((CHARTYPE *)"delimiter",word[0],9))
      option = STATE_DELIMITER;
   else
   {
      sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[0],lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if (num_params == 1)
   {
      if (option == STATE_SINGLE_QUOTE)
         parser->check_single_quote = TRUE;
      else if (option == STATE_DOUBLE_QUOTE)
         parser->check_double_quote = TRUE;
      else
      {
         sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
   }
   if (num_params == 2)
   {
      if (option == STATE_SINGLE_QUOTE
      &&  !equal((CHARTYPE *)"backslash",word[1],9))
      {
         sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[1],lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      else
      {
         parser->check_single_quote = TRUE;
         parser->backslash_single_quote = TRUE;
      }
      if (option == STATE_DOUBLE_QUOTE
      &&  !equal((CHARTYPE *)"backslash",word[1],9))
      {
         sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[1],lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      else
      {
         parser->check_double_quote = TRUE;
         parser->backslash_double_quote = TRUE;
      }
      if (option == STATE_DELIMITER
      &&  strlen((DEFCHAR *)word[1]) != 1)
      {
         sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[1],lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      else
      {
         parser->string_delimiter = word[1][0];
      }
   }
   if (num_params == 3)
   {
      if (option != STATE_DELIMITER)
      {
         sprintf((DEFCHAR *)tmp,"Too many tokens, line %d",lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      if (equal((CHARTYPE *)"backslash",word[2],9))
      {
         parser->string_delimiter = word[1][0];
         parser->backslash_delimiter = TRUE;
      }
      else
      {
         sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[2],lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
   }
   parser->have_string = TRUE;
   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_comment(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_comment(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
/*  Syntax:
 *         line x [column n | any | firstnonblank] [multiple | single]
 *         paired x y [nest | nonest] [multiple | single]
 */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CONCOM_PARAMS  5
   short rc=RC_OK;
   CHARTYPE *word[CONCOM_PARAMS+1];
   CHARTYPE strip[CONCOM_PARAMS];
   unsigned short num_params=0;
   short option=0,where=0;
   bool nested=FALSE;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_comment");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   strip[4]=STRIP_BOTH;
   num_params = param_split(line,word,CONCOM_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);

   if (equal((CHARTYPE *)"line",word[0],4))
      option = 1;
   else if (equal((CHARTYPE *)"paired",word[0],6))
      option = 2;
   else
   {
      sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[0],lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if (option == 1)
   {
      if (num_params < 3)
      {
         sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      if (strlen((DEFCHAR *)word[1]) > MAX_DELIMITER_LENGTH)
      {
         sprintf((DEFCHAR *)tmp,"Token too long '%s', line %d. Length should be <= %d",(DEFCHAR *)word[1],lineno,MAX_DELIMITER_LENGTH);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      if (equal((CHARTYPE *)"any",word[2],3))
      {
         if (num_params > 4)
         {
            sprintf((DEFCHAR *)tmp,"Too many tokens, line %d",lineno);
            display_error(216,(CHARTYPE *)tmp,FALSE);
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
         where = 0;
      }
      else if (equal((CHARTYPE *)"column",word[2],3))
      {
         if (!valid_positive_integer(word[3]))
         {
            sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[3],lineno);
            display_error(216,(CHARTYPE *)tmp,FALSE);
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
         where = atoi((DEFCHAR *)word[3]);
      }
      else if (equal((CHARTYPE *)"firstnonblank",word[2],13))
      {
         if (num_params > 4)
         {
            sprintf((DEFCHAR *)tmp,"Too many tokens, line %d",lineno);
            display_error(216,(CHARTYPE *)tmp,FALSE);
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
         where = MAX_INT;
      }
      else
      {
         sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",(DEFCHAR *)word[2],lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      parser->current_comments = parse_commentsll_add(parser->first_comments,parser->current_comments,sizeof(PARSE_COMMENTS));
      if (parser->first_comments == NULL)
         parser->first_comments = parser->current_comments;
      parser->current_comments->len_start_delim = strlen((DEFCHAR *)word[1]);
      strcpy((DEFCHAR *)parser->current_comments->start_delim,(DEFCHAR *)word[1]);
      parser->current_comments->column = where;
      parser->current_comments->line_comment = TRUE;
   }
   else
   {
      if (num_params < 3)
      {
         sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      switch(num_params)
      {
         case 4:
            if (equal((CHARTYPE *)"nest",word[3],4))
            {
               nested = TRUE;
            }
            else if (equal((CHARTYPE *)"nonest",word[3],6))
            {
               nested = FALSE;
            }
            else
            {
               sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[2],lineno);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return RC_INVALID_OPERAND;
            }
            /*
             * Fall through and setup the delimiters
             */
         case 3:
            if (strlen((DEFCHAR *)word[1]) > MAX_DELIMITER_LENGTH)
            {
               sprintf((DEFCHAR *)tmp,"Token too long '%s', line %d. Length should be <= %d",(DEFCHAR *)word[1],lineno,MAX_DELIMITER_LENGTH);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return RC_INVALID_OPERAND;
            }
            if (strlen((DEFCHAR *)word[2]) > MAX_DELIMITER_LENGTH)
            {
               sprintf((DEFCHAR *)tmp,"Token too long '%s', line %d. Length should be <= %d",(DEFCHAR *)word[2],lineno,MAX_DELIMITER_LENGTH);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return RC_INVALID_OPERAND;
            }
            /*
             * Everything is fine now, add the new entry to the linked list
             */
            parser->current_comments = parse_commentsll_add(parser->first_comments,parser->current_comments,sizeof(PARSE_COMMENTS));
            if (parser->first_comments == NULL)
               parser->first_comments = parser->current_comments;
            parser->current_comments->len_start_delim = strlen((DEFCHAR *)word[1]);
            strcpy((DEFCHAR *)parser->current_comments->start_delim,(DEFCHAR *)word[1]);
            parser->current_comments->len_end_delim = strlen((DEFCHAR *)word[2]);
            strcpy((DEFCHAR *)parser->current_comments->end_delim,(DEFCHAR *)word[2]);
            parser->current_comments->nested = nested;
            parser->current_comments->line_comment = FALSE;
            parser->have_paired_comments = TRUE;
            break;
         default:
            break;
      }
   }
   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_header(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_header(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
/*  Syntax:
 *         line x [column n | any | firstnonblank]
 */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CONHEA_PARAMS  4
   short rc=RC_OK;
   CHARTYPE *word[CONHEA_PARAMS+1];
   CHARTYPE strip[CONHEA_PARAMS];
   unsigned short num_params=0;
   short where=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_header");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   num_params = param_split(line,word,CONHEA_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);

   if (!equal((CHARTYPE *)"line",word[0],4))
   {
      sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[0],lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if (num_params < 3)
   {
      sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if (strlen((DEFCHAR *)word[1]) > MAX_DELIMITER_LENGTH)
   {
      sprintf((DEFCHAR *)tmp,"Token too long '%s', line %d. Length should be <= %d",(DEFCHAR *)word[1],lineno,MAX_DELIMITER_LENGTH);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if (equal((CHARTYPE *)"any",word[2],3))
   {
      if (num_params > 4)
      {
         sprintf((DEFCHAR *)tmp,"Too many tokens, line %d",lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      where = 0;
   }
   else if (equal((CHARTYPE *)"column",word[2],3))
   {
      if (!valid_positive_integer(word[3]))
      {
         sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[3],lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      where = atoi((DEFCHAR *)word[3]);
   }
   else if (equal((CHARTYPE *)"firstnonblank",word[2],13))
   {
      if (num_params > 4)
      {
         sprintf((DEFCHAR *)tmp,"Too many tokens, line %d",lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      where = MAX_INT;
   }
   else
   {
      sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",(DEFCHAR *)word[2],lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   parser->current_header = parse_headerll_add(parser->first_header,parser->current_header,sizeof(PARSE_HEADERS));
   if (parser->first_header == NULL)
      parser->first_header = parser->current_header;
   parser->current_header->len_header_delim = strlen((DEFCHAR *)word[1]);
   strcpy((DEFCHAR *)parser->current_header->header_delim,(DEFCHAR *)word[1]);
   parser->current_header->header_column = where;
   parser->have_headers = TRUE;
   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_label(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_label(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
/*  Syntax:
 *         delimiter x [column n | any | firstnonblank]
 *         column n
 */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CONLAB_PARAMS  4
   short rc=RC_OK;
   CHARTYPE *word[CONLAB_PARAMS+1];
   CHARTYPE strip[CONLAB_PARAMS];
   unsigned short num_params=0;
   short where=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_label");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   num_params = param_split(line,word,CONLAB_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);

   if (num_params < 2)
   {
      sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   /*
    * column n
    */
   if (equal((CHARTYPE *)"column",word[0],6))
   {
      if (!valid_positive_integer(word[1]))
      {
         sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",(DEFCHAR *)word[1],lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      where = atoi((DEFCHAR *)word[1]);
      parser->len_label_delim = 0;
      strcpy((DEFCHAR *)parser->label_delim,(DEFCHAR *)"");
   }
   else if (equal((CHARTYPE *)"delimiter",word[0],9))
   {
      if (num_params < 3)
      {
         sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      if (strlen((DEFCHAR *)word[1]) > MAX_DELIMITER_LENGTH)
      {
         sprintf((DEFCHAR *)tmp,"Token too long '%s', line %d. Length should be <= %d",(DEFCHAR *)word[1],lineno,MAX_DELIMITER_LENGTH);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      if (equal((CHARTYPE *)"any",word[2],3))
      {
         if (num_params > 4)
         {
            sprintf((DEFCHAR *)tmp,"Too many tokens, line %d",lineno);
            display_error(216,(CHARTYPE *)tmp,FALSE);
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
         where = 0;
      }
      else if (equal((CHARTYPE *)"column",word[2],3))
      {
         if (!valid_positive_integer(word[3]))
         {
            sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",(DEFCHAR *)word[3],lineno);
            display_error(216,(CHARTYPE *)tmp,FALSE);
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
         where = atoi((DEFCHAR *)word[3]);
      }
      else if (equal((CHARTYPE *)"firstnonblank",word[2],13))
      {
         if (num_params > 4)
         {
            sprintf((DEFCHAR *)tmp,"Too many tokens, line %d",lineno);
            display_error(216,(CHARTYPE *)tmp,FALSE);
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
         where = MAX_INT;
      }
      else
      {
         sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",(DEFCHAR *)word[2],lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      parser->len_label_delim = strlen((DEFCHAR *)word[1]);
      strcpy((DEFCHAR *)parser->label_delim,(DEFCHAR *)word[1]);
   }
   else
   {
      sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[0],lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   parser->label_column = where;
   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_match(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_match(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
/*  Syntax:
 *         ( )
 *         left,left right,right [middle,middle]
 */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_match");
   /*
    * Just set have_match to TRUE
    */
   parser->have_match = TRUE;
   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_keyword(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_keyword(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
/*  Syntax:
 *         keyword [TYPE n [ALTernate x]]
 *         keyword [ALTernate x [TYPE n]]
 */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CONKEY_PARAMS  5
   short rc=RC_OK;
   CHARTYPE *word[CONKEY_PARAMS+1];
   CHARTYPE strip[CONKEY_PARAMS];
   unsigned short num_params=0;
   CHARTYPE alternate=255;
   int i,num_pairs,keyword_idx,value_idx;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_keyword");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   strip[4]=STRIP_BOTH;
   num_params = param_split(line,word,CONKEY_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);

   if (num_params > 1)
   {
      /*
       * Total parameters must be a multiple of 2
       */
      if ( (num_params-1) % 2 != 0 )
      {
         sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",line,lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      num_pairs = (num_params-1) / 2;
      for ( i = 0; i < num_pairs; i++ )
      {
         keyword_idx = 1+(i*2);
         value_idx = 2+(i*2);
         if (equal((CHARTYPE *)"alternate",word[keyword_idx],3))
         {
            if (strlen((DEFCHAR *)word[value_idx]) > 1)
            {
               sprintf((DEFCHAR *)tmp,"ALTernate must be followed by a single character, line %d",lineno);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return RC_INVALID_OPERAND;
            }
            if (word[value_idx][0] >= '1' && word[value_idx][0] <= '9')
               alternate = word[value_idx][0] - '1' + 26;
            else if (word[value_idx][0] >= 'a' && word[value_idx][0] <= 'z')
               alternate = word[value_idx][0] - 'a';
            else if (word[value_idx][0] >= 'A' && word[value_idx][0] <= 'Z')
               alternate = word[value_idx][0] - 'A';
            else
            {
               sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[value_idx],lineno);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return(RC_INVALID_OPERAND);
            }
         }
         else if (equal((CHARTYPE *)"type",word[keyword_idx],3))
         {
         }
         else
         {
            sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[keyword_idx],lineno);
            display_error(216,(CHARTYPE *)tmp,FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
         }
      }
   }
   parser->current_keyword = parse_keywordll_add(parser->first_keyword,parser->current_keyword,sizeof(PARSE_KEYWORDS));
   if (parser->first_keyword == NULL)
      parser->first_keyword = parser->current_keyword;
   parser->current_keyword->keyword_length = strlen((DEFCHAR *)word[0]);
   parser->current_keyword->keyword = (CHARTYPE *)(*the_malloc)((1+parser->current_keyword->keyword_length)*sizeof(CHARTYPE));
   if (parser->current_keyword->keyword == NULL)
   {
      display_error(216,(CHARTYPE *)"out of memory",FALSE);
      TRACE_RETURN();
      return RC_OUT_OF_MEMORY;
   }
   strcpy((DEFCHAR *)parser->current_keyword->keyword,(DEFCHAR *)word[0]);
   parser->current_keyword->alternate = alternate;
   /*
    * Put keyword into hashtable - TBD
    */
   if (parser->current_keyword->keyword_length < parser->min_keyword_length)
      parser->min_keyword_length = parser->current_keyword->keyword_length;

   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_function(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_function(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CONFUN_PARAMS  5
   short rc=RC_OK;
   CHARTYPE *word[CONFUN_PARAMS+1];
   CHARTYPE strip[CONFUN_PARAMS+1];
   unsigned short num_params=0;
   CHARTYPE alternate=255;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_function");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   strip[4]=STRIP_BOTH;
   num_params = param_split(line,word,CONFUN_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params > 1)
   {
      if (equal((CHARTYPE *)"alternate",word[1],3))
      {
         if (num_params > 2)
         {
            if (strlen((DEFCHAR *)word[2]) > 1)
            {
               sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return RC_INVALID_OPERAND;
            }
            if (word[2][0] >= '1' && word[2][0] <= '9')
               alternate = word[2][0] - '1' + 26;
            else if (word[2][0] >= 'a' && word[2][0] <= 'z')
               alternate = word[2][0] - 'a';
            else if (word[2][0] >= 'A' && word[2][0] <= 'Z')
               alternate = word[2][0] - 'A';
            else
            {
               sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[2],lineno);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return(RC_INVALID_OPERAND);
            }
         }
         else
         {
            sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
            display_error(216,(CHARTYPE *)tmp,FALSE);
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
      }
   }

   parser->current_function = parse_functionll_add(parser->first_function,parser->current_function,sizeof(PARSE_FUNCTIONS));
   if (parser->first_function == NULL)
      parser->first_function = parser->current_function;
   parser->current_function->function_length = strlen((DEFCHAR *)word[0]);
   parser->current_function->function = (CHARTYPE *)(*the_malloc)((1+parser->current_function->function_length)*sizeof(CHARTYPE));
   if (parser->current_function->function == NULL)
   {
      display_error(216,(CHARTYPE *)"out of memory",FALSE);
      TRACE_RETURN();
      return RC_OUT_OF_MEMORY;
   }
   strcpy((DEFCHAR *)parser->current_function->function,(DEFCHAR *)word[0]);
   parser->current_function->alternate = alternate;
   /*
    * Put function into hashtable - TBD
    */
   if (parser->current_function->function_length < parser->min_function_length)
      parser->min_function_length = parser->current_function->function_length;

   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_option(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_option(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
/*  Syntax:
 *         REXX
 *         PREPROCESSOR char
 *         FUNCTION char BLANK|NOBLANK [* ALTernate x]
 */
/***********************************************************************/
{
#define CONOPT_PARAMS  6
   short rc=RC_OK;
   CHARTYPE *word[CONOPT_PARAMS+1];
   CHARTYPE strip[CONOPT_PARAMS];
   CHARTYPE alternate;
   unsigned short num_params=0;
   bool rexx_option=FALSE,preprocessor_option=FALSE,function_option=FALSE;

   TRACE_FUNCTION( "parser.c:  construct_option" );
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   strip[4]=STRIP_BOTH;
   strip[5]=STRIP_BOTH;
   num_params = param_split( line, word, CONOPT_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE );

   if ( equal( (CHARTYPE *)"rexx", word[0], 4 ) )
      rexx_option = TRUE;
   else if (equal( (CHARTYPE *)"preprocessor", word[0], 12 ) )
      preprocessor_option = TRUE;
   else if ( equal( (CHARTYPE *)"function", word[0], 8 ) )
      function_option = TRUE;
   else
   {
      sprintf( (DEFCHAR *)tmp, "Invalid operand '%s', line %d", word[0], lineno );
      display_error( 216, (CHARTYPE *)tmp, FALSE );
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if ( num_params == 1 )
   {
      if ( preprocessor_option
      ||   function_option )
      {
         sprintf( (DEFCHAR *)tmp, "Too few tokens, line %d", lineno );
         display_error( 216, (CHARTYPE *)tmp, FALSE );
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
   }
   else
   {
      if ( rexx_option )
      {
         sprintf( (DEFCHAR *)tmp, "Too many tokens, line %d", lineno );
         display_error( 216, (CHARTYPE *)tmp, FALSE );
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      else if ( preprocessor_option )
      {
         if ( num_params > 2 )
         {
            sprintf( (DEFCHAR *)tmp, "Too many tokens, line %d", lineno );
            display_error( 216, (CHARTYPE *)tmp, FALSE );
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
         else
         {
            if ( strlen( (DEFCHAR *)word[1]) != 1 )
            {
               sprintf( (DEFCHAR *)tmp, "Invalid operand '%s', line %d", word[1], lineno );
               display_error( 216, (CHARTYPE *)tmp, FALSE );
               TRACE_RETURN();
               return RC_INVALID_OPERAND;
            }
            parser->preprocessor_char = word[1][0];
         }
      }
      else if ( function_option )
      {
         if ( num_params < 3 )
         {
            sprintf( (DEFCHAR *)tmp, "Too few tokens, line %d", lineno );
            display_error( 216, (CHARTYPE *)tmp, FALSE );
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
         else
         {
            if ( strlen( (DEFCHAR *)word[1] ) != 1 )
            {
               sprintf( (DEFCHAR *)tmp, "Invalid operand '%s', line %d", word[1], lineno );
               display_error( 216, (CHARTYPE *)tmp, FALSE );
               TRACE_RETURN();
               return RC_INVALID_OPERAND;
            }
            if ( equal( (CHARTYPE *)"blank", word[2], 5 ) )
            {
               parser->function_blank = TRUE;
            }
            else if ( equal( (CHARTYPE *)"noblank", word[2], 7 ) )
            {
               parser->function_blank = FALSE;
            }
            else
            {
               sprintf( (DEFCHAR *)tmp, "Invalid operand '%s', line %d", word[1], lineno );
               display_error( 216, (CHARTYPE *)tmp, FALSE );
               TRACE_RETURN();
               return RC_INVALID_OPERAND;
            }
            parser->function_char = word[1][0];
            /*
             * The remainder of the checking is done for the optional
             * "* alternate x"
             */
            if ( num_params > 3 )
            {
               if ( num_params != 6 )
               {
                  sprintf( (DEFCHAR *)tmp, "Too few tokens, line %d", lineno );
                  display_error( 216, (CHARTYPE *)tmp, FALSE );
                  TRACE_RETURN();
                  return RC_INVALID_OPERAND;
               }
               /*
                * Must have "DEFAULT ALTernate x"
                */
               if ( !equal( (CHARTYPE *)"default", word[3], 7 ) )
               {
                  sprintf( (DEFCHAR *)tmp, "Invalid operand '%s', line %d", word[3], lineno );
                  display_error( 216, (CHARTYPE *)tmp, FALSE );
                  TRACE_RETURN();
                  return RC_INVALID_OPERAND;
               }
               if ( equal( (CHARTYPE *)"alternate", word[4], 3 ) )
               {
                  if ( strlen( (DEFCHAR *)word[5] ) > 1 )
                  {
                     sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d", word[5], lineno );
                     display_error( 216, (CHARTYPE *)tmp, FALSE );
                     TRACE_RETURN();
                     return RC_INVALID_OPERAND;
                  }
                  if ( word[5][0] >= '1' && word[5][0] <= '9' )
                     alternate = word[5][0] - '1' + 26;
                  else if ( word[5][0] >= 'a' && word[5][0] <= 'z' )
                     alternate = word[5][0] - 'a';
                  else if ( word[5][0] >= 'A' && word[5][0] <= 'Z' )
                     alternate = word[5][0] - 'A';
                  else
                  {
                     sprintf( (DEFCHAR *)tmp, "Invalid operand '%s', line %d", word[5], lineno );
                     display_error( 216, (CHARTYPE *)tmp, FALSE );
                     TRACE_RETURN();
                     return(RC_INVALID_OPERAND);
                  }
                  parser->function_option_alternate = alternate;
                  parser->have_function_option_alternate = TRUE;
               }
               else
               {
                  sprintf( (DEFCHAR *)tmp, "Invalid operand '%s', line %d", word[4], lineno );
                  display_error( 216, (CHARTYPE *)tmp, FALSE );
                  TRACE_RETURN();
                  return RC_INVALID_OPERAND;
               }
            }
         }
      }
   }
   if (preprocessor_option)
      parser->preprocessor_option = TRUE;
   else if (rexx_option)
      parser->rexx_option = TRUE;
   else if (function_option)
      parser->function_option = TRUE;
   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_identifier(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_identifier(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CONID_PARAMS  3
   short rc=RC_OK;
   CHARTYPE *word[CONID_PARAMS+1];
   CHARTYPE strip[CONID_PARAMS];
   unsigned short num_params=0;
   CHARTYPE *pattern=NULL,*ptr;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_identifier");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   num_params = param_split(line,word,CONID_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 2)
   {
      sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   pattern = (CHARTYPE *)(*the_malloc)((20+line_length)*sizeof(CHARTYPE));
   if (pattern == NULL)
   {
      display_error(216,(CHARTYPE *)"out of memory",FALSE);
      TRACE_RETURN();
      return RC_OUT_OF_MEMORY;
   }
   /*
    * Create the pattern buffer for keywords...
    */
   strcpy((DEFCHAR *)pattern,(DEFCHAR *)word[0]);
   strcat((DEFCHAR *)pattern,(DEFCHAR *)word[1]);
   strcat((DEFCHAR *)pattern,"*");
   if (num_params == 3)
   {
      /*
       * Append the optional last character RE to the end of the
       * body RE
       */
      strcat((DEFCHAR *)pattern,(DEFCHAR *)word[2]);
/*      strcat((DEFCHAR *)pattern,"."); */
   }
   memset(&parser->body_pattern_buffer,0,sizeof(struct re_pattern_buffer));
   ptr = (CHARTYPE *)re_compile_pattern((DEFCHAR *)pattern,0,strlen((DEFCHAR *)pattern),&parser->body_pattern_buffer);
   if (ptr)
   {
      /*
       * If ptr returns something, it is an error string
       */
      sprintf((DEFCHAR *)tmp,"%s in %s",ptr,pattern);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      (*the_free)(pattern);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   parser->have_body_pattern_buffer = TRUE;
   /*
    * Create the pattern buffer for functions...
    * If we said to in :option section
    */
   if (parser->function_option)
   {
      strcpy((DEFCHAR *)pattern,(DEFCHAR *)word[0]);
      strcat((DEFCHAR *)pattern,(DEFCHAR *)word[1]);
      strcat((DEFCHAR *)pattern,"*");
      if (parser->function_blank)
      {
         strcat((DEFCHAR *)pattern," *");
      }
      if (num_params == 3)
      {
         strcat((DEFCHAR *)pattern,(DEFCHAR *)word[2]);
         strcat((DEFCHAR *)pattern,"?");
      }
      tmp[0] = parser->function_char;
      tmp[1] = '\0';
      strcat((DEFCHAR *)pattern,(DEFCHAR *)tmp);
      memset(&parser->function_pattern_buffer,0,sizeof(struct re_pattern_buffer));
      ptr = (CHARTYPE *)re_compile_pattern((DEFCHAR *)pattern,0,strlen((DEFCHAR *)pattern),&parser->function_pattern_buffer);
      if (ptr)
      {
         /*
          * If ptr returns something, it is an error string
          */
         sprintf((DEFCHAR *)tmp,"%s in %s",ptr,pattern);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         (*the_free)(pattern);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
      }
      (*the_free)(pattern);
      parser->have_function_pattern_buffer = TRUE;
   }
   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_postcompare(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_postcompare(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CONPOST_PARAMS  4
   CHARTYPE *word[CONPOST_PARAMS+1];
   CHARTYPE strip[CONPOST_PARAMS+1];
   unsigned short num_params=0;
   CHARTYPE *pattern=NULL,*ptr;
   PARSE_POSTCOMPARE *curr;
   CHARTYPE alternate=255;
   struct re_pattern_buffer pattern_buffer;
   bool is_class_type;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_postcompare");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   num_params = param_split(line,word,CONPOST_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 2)
   {
      sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   /*
    * Check the optional alternate colour parameters first.
    */
   if ( num_params > 2 )
   {
      if (equal((CHARTYPE *)"alternate",word[2],3))
      {
         if (num_params > 3)
         {
            if (strlen((DEFCHAR *)word[3]) > 1)
            {
               sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return RC_INVALID_OPERAND;
            }
            if (word[3][0] >= '1' && word[3][0] <= '9')
               alternate = word[3][0] - '1' + 26;
            else if (word[3][0] >= 'a' && word[3][0] <= 'z')
               alternate = word[3][0] - 'a';
            else if (word[3][0] >= 'A' && word[3][0] <= 'Z')
               alternate = word[3][0] - 'A';
            else
            {
               sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[3],lineno);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return(RC_INVALID_OPERAND);
            }
         }
         else
         {
            sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
            display_error(216,(CHARTYPE *)tmp,FALSE);
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
      }
   }
   /*
    * Now determine if the type is CLASS or STRING...
    */
   if (equal((CHARTYPE *)"class",word[0],5))
   {
      /*
       * CLASS type - word[1] is RE
       */
      pattern = (CHARTYPE *)(*the_malloc)((20+line_length)*sizeof(CHARTYPE));
      if (pattern == NULL)
      {
         display_error(216,(CHARTYPE *)"out of memory",FALSE);
         TRACE_RETURN();
         return RC_OUT_OF_MEMORY;
      }
      /*
       * Create the pattern buffer for the RE...
       */
      strcpy((DEFCHAR *)pattern,(DEFCHAR *)word[1]);
      memset(&pattern_buffer,0,sizeof(struct re_pattern_buffer));
      ptr = (CHARTYPE *)re_compile_pattern((DEFCHAR *)pattern,0,strlen((DEFCHAR *)pattern),&pattern_buffer);
      if (ptr)
      {
         /*
          * If ptr returns something, it is an error string
          */
         sprintf((DEFCHAR *)tmp,"%s in %s",ptr,pattern);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         (*the_free)(pattern);
         TRACE_RETURN();
         return RC_INVALID_OPERAND;
       }
      (*the_free)(pattern);
      is_class_type = TRUE;
   }
   else if (equal((CHARTYPE *)"text",word[0],4))
   {
      /*
       * TEXT type - word[1] is string
       */
      is_class_type = FALSE;
   }
   else
   {
      sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[0],lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }

   curr = parse_postcomparell_add(parser->first_postcompare,parser->current_postcompare,sizeof(PARSE_POSTCOMPARE));
   if ( curr == NULL )
   {
      display_error(216,(CHARTYPE *)"out of memory",FALSE);
      TRACE_RETURN();
      return RC_OUT_OF_MEMORY;
   }
   parser->current_postcompare = curr;

   if (parser->first_postcompare == NULL)
      parser->first_postcompare = parser->current_postcompare;

   parser->current_postcompare->string_length = 0;
   parser->current_postcompare->string = NULL;
   if ( is_class_type )
   {
      parser->current_postcompare->pattern_buffer = pattern_buffer;
   }
   else
   {
      parser->current_postcompare->string_length = strlen((DEFCHAR *)word[1]);
      parser->current_postcompare->string = (CHARTYPE *)(*the_malloc)((1+parser->current_postcompare->string_length)*sizeof(CHARTYPE));
      if (parser->current_postcompare->string == NULL)
      {
         display_error(216,(CHARTYPE *)"out of memory",FALSE);
         TRACE_RETURN();
         return RC_OUT_OF_MEMORY;
      }
      strcpy((DEFCHAR *)parser->current_postcompare->string,(DEFCHAR *)word[1]);
   }

   parser->current_postcompare->alternate = alternate;
   parser->current_postcompare->is_class_type = is_class_type;
   parser->have_postcompare = TRUE;
   TRACE_RETURN();
   return (RC_OK);
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_number(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_number(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
{
#define CONNUM_PARAMS  1
   CHARTYPE *word[CONNUM_PARAMS+1];
   CHARTYPE strip[CONNUM_PARAMS+1];
   unsigned short num_params=0;
   DEFCHAR *pattern;
   char *ptr;

   TRACE_FUNCTION("parser.c:  construct_number");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   num_params = param_split(line,word,CONNUM_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 0)
   {
      sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   if (num_params > 1)
   {
      sprintf((DEFCHAR *)tmp,"Too many tokens, line %d",lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   /*
    * Get number type
    */
   if ( equal( (CHARTYPE *)"rexx", word[0], 4 ) )
   {
      pattern = REXX_NUMBER_RE;
   }
   else if (equal( (CHARTYPE *)"cobol", word[0], 5 ) )
   {
      pattern = COBOL_NUMBER_RE;
   }
   else if ( equal( (CHARTYPE *)"c", word[0], 1 ) )
   {
      pattern =  C_NUMBER_RE;
   }
   else
   {
      sprintf( (DEFCHAR *)tmp, "Invalid operand '%s', line %d", word[0], lineno );
      display_error( 216, (CHARTYPE *)tmp, FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Create the pattern buffer for the RE...
    */
   memset( &parser->number_pattern_buffer, 0, sizeof(struct re_pattern_buffer) );
   ptr = (DEFCHAR *)re_compile_pattern( pattern, 0, strlen( pattern), &parser->number_pattern_buffer );
   if (ptr)
   {
      /*
       * If ptr returns something, it is an error string
       */
      sprintf( (DEFCHAR *)tmp, "%s in %s", ptr, pattern );
      display_error( 216, (CHARTYPE *)tmp, FALSE );
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }

   parser->have_number_pattern_buffer = TRUE;
   TRACE_RETURN();
   return (RC_OK);
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
static short construct_column(CHARTYPE *line, int line_length, PARSER_DETAILS *parser, int lineno)
#else
static short construct_column(line, line_length, parser, lineno)
CHARTYPE *line;
int line_length;
PARSER_DETAILS *parser;
int lineno;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define CONCOL_PARAMS  5
   CHARTYPE *word[CONCOL_PARAMS+1];
   CHARTYPE strip[CONCOL_PARAMS+1];
   unsigned short num_params=0;
   CHARTYPE alternate=255;
   LINETYPE col1=0L,col2=0L;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_column");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   strip[4]=STRIP_BOTH;
   strip[5]=STRIP_BOTH;
   num_params = param_split(line,word,CONCOL_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 3)
   {
      sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return RC_INVALID_OPERAND;
   }
   /*
    * Check the optional alternate colour parameters first.
    */
   if ( num_params > 3 )
   {
      if (equal((CHARTYPE *)"alternate",word[3],3))
      {
         if (num_params > 3)
         {
            if (strlen((DEFCHAR *)word[4]) > 1)
            {
               sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return RC_INVALID_OPERAND;
            }
            if (word[4][0] >= '1' && word[4][0] <= '9')
               alternate = word[4][0] - '1' + 26;
            else if (word[4][0] >= 'a' && word[4][0] <= 'z')
               alternate = word[4][0] - 'a';
            else if (word[4][0] >= 'A' && word[4][0] <= 'Z')
               alternate = word[4][0] - 'A';
            else
            {
               sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[4],lineno);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               TRACE_RETURN();
               return(RC_INVALID_OPERAND);
            }
         }
         else
         {
            sprintf((DEFCHAR *)tmp,"Too few tokens, line %d",lineno);
            display_error(216,(CHARTYPE *)tmp,FALSE);
            TRACE_RETURN();
            return RC_INVALID_OPERAND;
         }
      }
   }
   /*
    * 'EXCLUDE' is only valid option at this stage
    */
   if (!equal((CHARTYPE *)"exclude",word[0],7))
   {
      sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[0],lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Now obtain the start and end columns.
    */
   if (!valid_positive_integer(word[1]))
   {
      sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[1],lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   col1 = atol((DEFCHAR *)word[1]);
   /*
    * Second parameter must be positive integer or *
    */
   if (strcmp((DEFCHAR *)word[2],"*") == 0)
      col2 = max_line_length;
   else
   {
      if (!valid_positive_integer(word[2]))
      {
         sprintf((DEFCHAR *)tmp,"Invalid operand '%s', line %d",word[2],lineno);
         display_error(216,(CHARTYPE *)tmp,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      col2 = atol((DEFCHAR *)word[2]);
   }

   if (col2 > max_line_length)
      col2 = max_line_length;
   if (col1 > col2)
   {
      sprintf((DEFCHAR *)tmp,"First column(%ld) must be < last column(%ld), line %d",col1,col2,lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if ( parser->number_columns == MAX_PARSER_COLUMNS )
   {
      sprintf((DEFCHAR *)tmp,"Maximum number of :column options (%d) exceeded, line %d",MAX_PARSER_COLUMNS,lineno);
      display_error(216,(CHARTYPE *)tmp,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   parser->first_column[parser->number_columns] = col1;
   parser->last_column[parser->number_columns] = col2;
   parser->column_alternate[parser->number_columns] = alternate;
   parser->number_columns++;
   parser->have_columns = TRUE;

   TRACE_RETURN();
   return (RC_OK);
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
short construct_parser(CHARTYPE *contents, int contents_length, PARSER_DETAILS **parser, CHARTYPE *parser_name, CHARTYPE *filename)
#else
short construct_parser(contents, contents_length, parser, parser_name, filename)
CHARTYPE *contents;
int contents_length;
PARSER_DETAILS **parser;
CHARTYPE *parser_name,*filename;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   int i,j,line_len;
   int state=0;
   CHARTYPE _THE_FAR line[250];
   short rc=RC_OK;
   int lineno=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  construct_parser");
   last_parser = parserll_add(first_parser,last_parser,sizeof(PARSER_DETAILS));
   if (first_parser == NULL)
      first_parser = last_parser;
   strcpy((DEFCHAR *)last_parser->parser_name,(DEFCHAR *)parser_name);
   strcpy((DEFCHAR *)last_parser->filename,(DEFCHAR *)filename);
   last_parser->min_keyword_length = MAX_INT;
   last_parser->min_function_length = MAX_INT;
   *parser = last_parser;
   for (i=0,j=0;i<contents_length;i++)
   {
      if (contents[i] == '\n')
      {
         /*
          * We have a line, parse it
          */
         lineno++;
         line[j] = '\0';
         j = 0;
         strtrunc(line);
         line_len = strlen((DEFCHAR *)line);
         /*
          * Ignore blank lines and comments
          */
         if (line_len == 0
         ||  line[0] == '*')
            continue;
         if (line[0] == ':')
            state = STATE_FILE_HEADER;
         switch(state)
         {
            case STATE_FILE_HEADER:
               if (memcmpi(line,(CHARTYPE *)":case",line_len) == 0)
               {
                  state = STATE_CASE;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":option",line_len) == 0)
               {
                  state = STATE_OPTION;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":number",line_len) == 0)
               {
                  state = STATE_NUMBER;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":identifier",line_len) == 0)
               {
                  state = STATE_IDENTIFIER;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":string",line_len) == 0)
               {
                  state = STATE_STRING;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":comment",line_len) == 0)
               {
                  state = STATE_COMMENT;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":header",line_len) == 0)
               {
                  state = STATE_HEADER;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":label",line_len) == 0)
               {
                  state = STATE_LABEL;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":match",line_len) == 0)
               {
                  state = STATE_MATCH;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":keyword",line_len) == 0)
               {
                  state = STATE_KEYWORD;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":function",line_len) == 0)
               {
                  state = STATE_FUNCTION;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":postcompare",line_len) == 0)
               {
                  state = STATE_POSTCOMPARE;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":column",line_len) == 0)
               {
                  state = STATE_COLUMN;
                  break;
               }
               if (memcmpi(line,(CHARTYPE *)":markup",line_len) == 0)
               {
                  state = STATE_MARKUP;
                  break;
               }
               sprintf((DEFCHAR *)tmp,"invalid header: %s",line);
               display_error(216,(CHARTYPE *)tmp,FALSE);
               rc = RC_INVALID_OPERAND;
               break;
            case STATE_CASE:
               rc = construct_case(line,line_len,last_parser,lineno);
               break;
            case STATE_OPTION:
               rc = construct_option(line,line_len,last_parser,lineno);
               break;
            case STATE_NUMBER:
               rc = construct_number(line,line_len,last_parser,lineno);
               break;
            case STATE_IDENTIFIER:
               rc = construct_identifier(line,line_len,last_parser,lineno);
               break;
            case STATE_STRING:
               rc = construct_string(line,line_len,last_parser,lineno);
               break;
            case STATE_COMMENT:
               rc = construct_comment(line,line_len,last_parser,lineno);
               break;
            case STATE_HEADER:
               rc = construct_header(line,line_len,last_parser,lineno);
               break;
            case STATE_LABEL:
               rc = construct_label(line,line_len,last_parser,lineno);
               break;
            case STATE_MATCH:
               rc = construct_match(line,line_len,last_parser,lineno);
               break;
            case STATE_KEYWORD:
               rc = construct_keyword(line,line_len,last_parser,lineno);
               break;
            case STATE_FUNCTION:
               rc = construct_function(line,line_len,last_parser,lineno);
               break;
            case STATE_POSTCOMPARE:
               rc = construct_postcompare(line,line_len,last_parser,lineno);
               break;
            case STATE_COLUMN:
               rc = construct_column(line,line_len,last_parser,lineno);
               break;
            case STATE_MARKUP:
               rc = construct_markup(line,line_len,last_parser,lineno);
               break;
            default:
               break;
         }
         if (rc != RC_OK)
            break;
      }
      else
         line[j++] = contents[i];
   }
   TRACE_RETURN();
   return rc;
}
  
/***********************************************************************/
#ifdef HAVE_PROTO
short destroy_parser(PARSER_DETAILS *parser)
#else
short destroy_parser(parser)
PARSER_DETAILS *parser;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  destroy_parser");
   if (parser->first_comments)
   {
      parser->first_comments = parse_commentsll_free(parser->first_comments);
   }
   if (parser->first_keyword)
   {
      parser->first_keyword = parse_keywordll_free(parser->first_keyword);
   }
   if (parser->first_function)
   {
      parser->first_function = parse_functionll_free(parser->first_function);
   }
   if (parser->first_header)
   {
      parser->first_header = parse_headerll_free(parser->first_header);
   }
   if (parser->have_body_pattern_buffer)
   {
      the_regfree(&parser->body_pattern_buffer);
   }
   if (parser->have_function_pattern_buffer)
   {
      the_regfree(&parser->function_pattern_buffer);
   }
   if (parser->have_postcompare)
   {
      parser->first_postcompare = parse_postcomparell_free(parser->first_postcompare);
   }
   TRACE_RETURN();
   return rc;
}

/***********************************************************************/
#ifdef HAVE_PROTO
bool find_parser_mapping(FILE_DETAILS *fd, PARSER_MAPPING *curr_mapping)
#else
bool find_parser_mapping(fd,curr_mapping)
FILE_DETAILS *fd;
PARSER_MAPPING *curr_mapping;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  find_parser_mapping");
   if (curr_mapping->filemask)
   {
#ifdef UNIX
      if (thematch(curr_mapping->filemask,fd->fname,0) == 0)
#else
      if (thematch(curr_mapping->filemask,fd->fname,FNM_IGNORECASE) == 0)
#endif
      {
         TRACE_RETURN();
         return TRUE;
      }
   }
   else
   {
      /*
       * We now want to try "magic numbers" of shell scripts
       * If the file has at least one line, and that line starts
       * with #!, get the last string from the first word. This should
       * be the interpreter to use. eg #!/usr/bin/csh
       */
      if (fd
      &&  fd->first_line
      &&  fd->first_line->next)
      {
         LINE *curr=fd->first_line->next;
         CHARTYPE *ptr=curr->line;
         int i,state=STATE_START,start=0,len;
         if (curr->length > 3
         && *(curr->line) == '#'
         &&  *(curr->line+1) == '!')
         {
            for (i=2;i<curr->length+1;i++)
            {
               switch(state)
               {
                  case STATE_START:
                     if (*(ptr+i) == '/')
                     {
                        state = STATE_SLASH;
                        start = i;
                        break;
                     }
                     break;
                  case STATE_SLASH:
                     if (*(ptr+i) == '/')
                     {
                        start = i;
                        break;
                     }
                     if (*(ptr+i) == ' '
                     ||  *(ptr+i) == '\0')
                     {
                        len = i - start - 1;
                        if (len == curr_mapping->magic_number_length
                        &&  memcmp(ptr+start+1,curr_mapping->magic_number,len) == 0)
                        {
                           TRACE_RETURN();
                           return TRUE;
                           break;
                        }
                     }
                     break;
               }
            }
         }
      }
   }
   TRACE_RETURN();
   return FALSE;
}

/***********************************************************************/
#ifdef HAVE_PROTO
PARSER_DETAILS *find_auto_parser(FILE_DETAILS *fd)
#else
PARSER_DETAILS *find_auto_parser(fd)
FILE_DETAILS *fd;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   PARSER_MAPPING *curr_mapping=first_parser_mapping;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  find_auto_parser");
   /*
    * Check the filename against the available parsers' filemasks.
    * If the filemask uses filenames then check the filename. 
    * If the filemask uses "magic" check the magic number of the first
    * line of the file.                                                  
    */
   for(;curr_mapping!=NULL;curr_mapping=curr_mapping->next)
   {
      if (find_parser_mapping(fd,curr_mapping))
      {
         fd->parser = curr_mapping->parser;
         break;
      }
   }
   TRACE_RETURN();
   return(fd->parser);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short parse_reserved_line(RESERVED *rsrvd)
#else
short parse_reserved_line(rsrvd)
RESERVED *rsrvd;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   int i=0,j=0,k=0;
   chtype current_colour=set_colour(rsrvd->attr);
   int state=STATE_START;
   CHARTYPE *line=rsrvd->line;
   CHARTYPE *disp=rsrvd->disp;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("parser.c:  parse_reserved_line");
   if (CTLCHARx)
   {
      while(1)
      {
         switch(state)
         {
            case STATE_START:
               if (line[i] == ctlchar_escape)
               {
                  state = STATE_DELIMITER;
                  i++;
                  break;
               }
               rsrvd->highlighting[k] = current_colour;
               disp[k] = line[i];
               i++;
               k++;
               break;
            case STATE_DELIMITER:
               for (j=0;j<MAX_CTLCHARS;j++)
               {
                  if (ctlchar_char[j] == line[i])
                  {
                     current_colour = set_colour(&ctlchar_attr[j]);
                     state = STATE_START;
                     i++;
                     break;
                  }
               }
               rsrvd->highlighting[k] = current_colour;
               disp[k] = line[i];
               i++;
               k++;
               break;
         }
         if (i > rsrvd->length
         ||  i > 260)
            break;
      }
      rsrvd->disp_length = k-1;
   }
   else
   {
      memcpy(rsrvd->disp,rsrvd->line,rsrvd->length);
      for (i=0;i<rsrvd->length || i<260;i++)
      {
         rsrvd->highlighting[i] = current_colour;
      }
      rsrvd->disp_length = rsrvd->length;
   }
   TRACE_RETURN();
   return (RC_OK);
}
