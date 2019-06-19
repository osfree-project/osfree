/***********************************************************************/
/* trace.c - Debugging and tracing functions.                          */
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

static char RCSid[] = "$Id: trace.c,v 1.3 2001/12/18 08:23:27 mark Exp $";

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifdef HAVE_PROTO
void trace_initialise(void);
void trace_function(char *);
void trace_return(void);
void trace_string(char *,...);
void trace_constant(char *);
#else
void trace_initialise(/* void */);
void trace_function(/* char* */);
void trace_return(/* void */);
void trace_string(/* char*,... */);
void trace_constant(/* char * */);
#endif

/*--------------------------- global data -----------------------------*/
static char trace_save_str[40];
static char trace_env[40];
static FILE *trace_fp;
static short trace_number=0,trace_level=(-1);
/***********************************************************************/
#ifdef HAVE_PROTO
void trace_initialise(void)
#else
void trace_initialise()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 char *trace_env_ptr=getenv("THE_TRACE");
/*--------------------------- processing ------------------------------*/

 trace_fp = NULL;
 if (trace_env_ptr == NULL)
    return;
 strcpy(trace_env,trace_env_ptr);
 trace_number = trace_level = 0;
 strcpy(trace_save_str,"");
 return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void trace_function(char *trace_str)
#else
void trace_function(trace_str)
char *trace_str;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register int i;
   time_t timer;
   struct tm *tblock=NULL;
/*--------------------------- processing ------------------------------*/
   if (trace_level == (-1))
      return;
   trace_level++;
   trace_fp = fopen(trace_env,"a");
   for (i=0;i<trace_level;i++)
      fprintf(trace_fp,"  ");
   fprintf(trace_fp,"(%d)%-s",trace_level,trace_str);
   for (i=0;i<100-(trace_level*2)-strlen(trace_str);i++)
      fprintf(trace_fp," ");
   timer = time(NULL);
   tblock = localtime(&timer);
   fprintf(trace_fp,"%2d:%2d:%2d\n",tblock->tm_hour,tblock->tm_min,tblock->tm_sec);
   fclose(trace_fp);
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void trace_return(void)
#else
void trace_return()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 if (trace_level == (-1))
    return;
 trace_level--;
 if (trace_level < 0)
    fprintf(trace_fp,"****** trace level below zero ********");

 return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void trace_string(char *fmt,...)
#else
void trace_string(fmt)
char *fmt;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 va_list args;
/*--------------------------- processing ------------------------------*/
 va_start(args,fmt);
 trace_fp = fopen(trace_env,"a");
 if (trace_fp == NULL)
   {
    va_end(args);
    return;
   }
 vfprintf(trace_fp,fmt,args);
 fclose(trace_fp);
 va_end(args);
 return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void trace_constant(char *str)
#else
void trace_constant(str)
char *str;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 trace_fp = fopen(trace_env,"a");
 if (trace_fp == NULL)
    return;
 fprintf(trace_fp,"%s",str);
 fclose(trace_fp);
 return;
}
