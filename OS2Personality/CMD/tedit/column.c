/***********************************************************************/
/* COLUMN.C - Column commands                                          */
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

static char RCSid[] = "$Id: column.c,v 1.3 2001/12/18 08:23:26 mark Exp $";

#include <the.h>
#include <proto.h>

/***********************************************************************/
#ifdef HAVE_PROTO
short column_command(CHARTYPE *cmd_text,int cmd_type)
#else
short column_command(cmd_text,cmd_type)
CHARTYPE *cmd_text;
int cmd_type;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LENGTHTYPE i=0;
 LINETYPE true_line=0L;
 short rc=RC_OK;
 short len_params=0;
 unsigned short y=0,x=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("column.c:  column_command");
/*---------------------------------------------------------------------*/
/* All column commands under XEDIT compatibility refer to current line.*/
/* ******* At this stage, revert to THE behaviour at all times ******* */
/*---------------------------------------------------------------------*/
 if (compatible_feel == COMPAT_XEDIT)
    true_line = CURRENT_VIEW->current_line;
 else
    true_line = get_true_line(TRUE);
/*---------------------------------------------------------------------*/
/* If on TOF or BOF, exit with error.                                  */
/*---------------------------------------------------------------------*/
 if (TOF(true_line)
 ||  BOF(true_line))
   {
    display_error(36,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_NO_LINES_CHANGED);
   }
/*---------------------------------------------------------------------*/
/* If HEX mode is on, convert the hex string...                        */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->hex)
   {
    if ((len_params = convert_hex_strings(cmd_text)) == (-1))
      {
       display_error(32,(CHARTYPE *)"",FALSE);
       TRACE_RETURN();
       return(RC_INVALID_OPERAND);
      }
   }
 else
   len_params = strlen((DEFCHAR *)cmd_text);
/*---------------------------------------------------------------------*/
/* If on command line, copy current line into rec                      */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->current_window == WINDOW_COMMAND
 ||  compatible_feel == COMPAT_XEDIT)
   {
    post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->current_line,(LINE *)NULL);
    x = CURRENT_VIEW->current_column-1;
   }
 else
   {
    if (CURRENT_VIEW->current_window == WINDOW_PREFIX)
      {
       if (cmd_type != COLUMN_CAPPEND)
         {
          display_error(36,(CHARTYPE *)"",FALSE);
          TRACE_RETURN();
          return(RC_NO_LINES_CHANGED);
         }
      }
    if (curses_started)
       getyx(CURRENT_WINDOW,y,x);
    x = CURRENT_VIEW->verify_col-1+x;
   }
 switch(cmd_type)
   {
    case COLUMN_CAPPEND:
         CURRENT_VIEW->current_column = rec_len+1;
         for (i=0;i<len_params;i++)
           {
            if (rec_len > max_line_length)
               break;
            rec[rec_len] = *(cmd_text+i);
            rec_len++;
           }
#if 0
         rec[rec_len] = '\0';
#endif
         break;
    case COLUMN_CINSERT:
         if (x > rec_len)
           {
            rec_len = x;
            for (i=0;i<len_params;i++)
              {
               if (rec_len > max_line_length)
                  break;
               rec[rec_len] = *(cmd_text+i);
               rec_len++;
              }
#if 0
            rec[rec_len] = '\0';
#endif
           }
         else
           {
            rec = meminsmem(rec,cmd_text,len_params,x,max_line_length,rec_len);
            rec_len = min(max_line_length,rec_len+len_params);
#if 0
            rec[rec_len] = '\0';
#endif
           }
         break;
    case COLUMN_COVERLAY:
         for (i=0;i<len_params;i++)
           {
            if (x > max_line_length)
               break;
            switch(*(cmd_text+i))
              {
               case '_':
                    rec[x] = ' ';
                    break;
               case ' ':
                    break;
               default:
                    rec[x] = *(cmd_text+i);
                    break;
              }
            x++;
           }
         rec_len = max(rec_len,x+1);
#if 0
         rec[rec_len] = '\0';
#endif
         break;
    case COLUMN_CREPLACE:
         for (i=0;i<len_params;i++)
           {
            if (x > max_line_length)
               break;
            rec[x] = *(cmd_text+i);
            x++;
           }
         rec_len = max(rec_len,x+1);
#if 0
         rec[rec_len] = '\0';
#endif
         break;
   }
 if (CURRENT_VIEW->current_window == WINDOW_COMMAND
 ||  compatible_feel == COMPAT_XEDIT)
   {
    post_process_line(CURRENT_VIEW,CURRENT_VIEW->current_line,(LINE *)NULL,TRUE);
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   }
 else
   {
    switch(cmd_type)
      {
       case COLUMN_CAPPEND:
            if (CURRENT_VIEW->current_window == WINDOW_PREFIX)
              {
               CURRENT_VIEW->current_window = WINDOW_FILEAREA;
               if (curses_started)
                  wmove(CURRENT_WINDOW,y,0);
              }
            rc = execute_move_cursor(CURRENT_VIEW->current_column-1);
            break;
       case COLUMN_CINSERT:
            break;
      }
   }
 build_screen(current_screen);
 display_screen(current_screen);
 TRACE_RETURN();
 return(rc);
}
