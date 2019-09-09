/***********************************************************************/
/* MANEXT - Extract manual pages from C source code.                   */
/***********************************************************************/
/*
 * MANEXT - A program to extract manual pages from C source code.
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

static char RCSid[] = "$Id: manext.c,v 1.2 2001/12/18 08:23:27 mark Exp $";

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_PROTO
void display_info(void);
#else
void display_info();
#endif

typedef unsigned char bool;
typedef char CHARTYPE;

#define TRUE    1
#define FALSE   0

#define MAX_LINE 255

#define FORMAT_MANUAL     0
#define FORMAT_QUICK_REF  1

#define STATE_IGNORE    0
#define STATE_SYNTAX    1
#define STATE_COMMAND   2
#define STATE_DEFAULT   3
/***********************************************************************/
#ifdef HAVE_PROTO
short strzne(CHARTYPE *str,CHARTYPE ch)
#else
short strzne(str,ch)
CHARTYPE *str;
CHARTYPE ch;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short len=0;
 register short  i = 0;
/*--------------------------- processing ------------------------------*/
 len = strlen(str);
 for (; i<len && str[i]==ch; i++);
 if (i>=len)
    i = (-1);
 return(i);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short strzrevne(CHARTYPE *str,CHARTYPE ch)
#else
short strzrevne(str,ch)
CHARTYPE *str;
CHARTYPE ch;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short len=0;
/*--------------------------- processing ------------------------------*/
 len = strlen(str);
 for (--len; len>=0 && str[len]==ch; len--);
 return(len);
}
/***********************************************************************/
#ifdef HAVE_PROTO
bool blank_field(CHARTYPE *field)
#else
bool blank_field(field)
CHARTYPE *field;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 if (strzne(field,' ') == (-1))
    return(TRUE);                /* field is NULL or just contains spaces */
 return(FALSE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *strtrunc(CHARTYPE *string)
#else
CHARTYPE *strtrunc(string)
CHARTYPE *string;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short pos=0;
/*--------------------------- processing ------------------------------*/
 pos = strzrevne(string,' ');
 if (pos == (-1))
    strcpy(string,"");
 else
    *(string+pos+1) = '\0';
 pos = strzne(string,' ');
 if (pos != (-1))
   {
    for (i=0;*(string+i+pos)!='\0';i++)
       *(string+i) = *(string+i+pos);
    *(string+i) = '\0';
   }
 return(string);
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *strtrim(CHARTYPE *string,char ch)
#else
CHARTYPE *strtrim(string,ch)
CHARTYPE *string,ch;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short pos=0;
/*--------------------------- processing ------------------------------*/
 for (i=0;i<strlen(string);i++)
   {
    if (*(string+i) == ch)
       return(string+i+1);
   }
 return(string);
}
/***********************************************************************/
#ifdef HAVE_PROTO
int main(int argc,char *argv[])
#else
int main(argc,argv)
int argc;
char *argv[];
#endif
/***********************************************************************/
{
 char    s[MAX_LINE + 1];        /* input line */
 char    save_line[MAX_LINE + 1];
 register int     i = 0;
 FILE *fp;
 char c;
 char append=0;
 int format=FORMAT_MANUAL;
 int state=STATE_IGNORE;
 int file_start=1;

#ifdef __EMX__
 _wildcard(&argc,&argv);
#endif

 if (strcmp(argv[1],"-h") == 0)
   {
    display_info();
    exit(1);
   }
 if (strcmp(argv[1],"-q") == 0) /* generate quick reference */
   {
    format = FORMAT_QUICK_REF;
    file_start = 2;
   }
 for(i=file_start;i<argc;i++)
    {
     if ((fp = fopen(argv[i],"r")) == NULL)
       {
        fprintf(stderr,"\nCould not open %s\n",argv[i]);
        continue;
       }
     while(1)
       {
        if (fgets(s, (int)sizeof(s), fp) == NULL)
          {
           if (ferror(fp) != 0)
             {
              fprintf(stderr, "*** Error reading %s.  Exiting.\n",argv[i]);
              exit(1);
             }
           break;
          }

        /* check for manual entry marker at beginning of line */
        if (strncmp(s, "/*man-start*", 12) != 0)
            continue;

        state = STATE_IGNORE;
        /* inner loop */
        for (;;)
           {
            /* read next line of manual entry */
            if (fgets(s, (int)sizeof(s), fp) == NULL)
              {
               if (ferror(fp) != 0)
                 {
                  fprintf(stderr, "*** Error reading %s.  Exiting.\n",argv[i]);
                  exit(1);
                 }
                break;
              }
            /* check for end of entry marker */
            if (strncmp(s, "**man-end", 9) == 0)
               break;
            switch(format)
              {
               case FORMAT_MANUAL:
                    printf("     %s",s);
                    break;
               case FORMAT_QUICK_REF:
                    s[strlen(s)-1] = '\0';
                    switch(state)
                      {
                       case STATE_IGNORE:
                            if (strncmp(s, "COMMAND", 7) == 0)
                              {
                               state = STATE_COMMAND;
                               break;
                              }
                            if (strncmp(s, "SYNTAX", 6) == 0)
                              {
                               state = STATE_SYNTAX;
                               break;
                              }
                            if (strncmp(s, "DEFAULT", 7) == 0)
                              {
                               state = STATE_DEFAULT;
                               break;
                              }
                            break;
                       case STATE_COMMAND:
                            strcpy(save_line,s);
                            state = STATE_IGNORE;
                            break;
                       case STATE_DEFAULT:
                            if (blank_field(s))
                              {
                               state = STATE_IGNORE;
                               break;
                              }
                            printf("       Default: %s\n",strtrunc(s));
                            break;
                       case STATE_SYNTAX:
                            if (blank_field(s))
                              {
                               printf("       %s\n",strtrunc(strtrim(save_line,'-')));
                               state = STATE_IGNORE;
                               break;
                              }
                            printf(" %s\n",strtrunc(s));
                            break;
                       default:
                            break;
                      }
                    break;
               default:
                    break;
              }
            }
        if (format == FORMAT_MANUAL)
           printf("\n\n\n     --------------------------------------------------------------------------\n");

        /* check if end of file */
        if (feof(fp) != 0)
            break;
       }
     fclose(fp);
    }
 if (format == FORMAT_MANUAL)
    printf("\n\n\n\n\n");
 return(0);
}
/***********************************************************************/
#ifdef HAVE_PROTO
void display_info(void)
#else
void display_info()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/

 fprintf(stderr,"\nMANEXT 1.00 Copyright (C) 1991-1999 Mark Hessling\n");
 fprintf(stderr,"All rights reserved.\n");
 fprintf(stderr,"MANEXT is distributed under the terms of the GNU\n");
 fprintf(stderr,"General Public License and comes with NO WARRANTY.\n");
 fprintf(stderr,"See the file COPYING for details.\n");
 fprintf(stderr,"\nUsage: MANEXT sourcefile [...]\n\n");
 fflush(stderr);
 return;
}
