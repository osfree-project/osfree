/* Unix screen update functions for regutil
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is regutil.
 *
 * The Initial Developer of the Original Code is Patrick TJ McPhee.
 * Portions created by Patrick McPhee are Copyright © 1998, 2001
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/regscreenux.c,v 1.3 2004/08/21 14:48:40 prokushev Exp $
 */
#include "regutil.h"
#ifdef USE_TERMCAP_DB
# ifdef USE_TERM_H
#  include <sys/ioctl.h>
#  include <curses.h>
#  include <term.h>
# else
#  include <termcap.h>
# endif
#endif
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>

/* sys/select.h is not present on hp-ux */
#ifndef __hpux
# include <sys/select.h>
#endif

/* ******************************************************************** */
/* ************************** Screen Update *************************** */
/* ******************************************************************** */

/* using termcap */

static const char notimp[] = "not implemented";
#define what() memcpy(result->strptr, notimp, sizeof(notimp)-1), result->strlength = sizeof(notimp)-1


/* load the termcap info */
#ifdef USE_TERMCAP_DB
static void sethandles(void)
{
    static char tcent[1024];

    if (!*tcent)
      tgetent(tcent, getenv("TERM"));
}
#endif


/* syscls() */
rxfunc(syscls)
{
#ifdef USE_TERMCAP_DB
   static char clrbuf[100], * clr = clrbuf;

   if (!*clr) {
      sethandles();
      clr = tgetstr("cl", &clr);
   }
#else
   /* ansi terminal control for clearing the screen should work with any
    * terminal these days... */
   static const char clr[] = "\033[2J";
#endif

   if (clr) {
      fputs(clr, stdout);
      fflush(stdout);
      result_zero();
   }
   else {
       result_one();
   }

   return 0;
} 


/* syscurpos([row],[column]) -- returns current position, but I don't know
 * how to get that */
rxfunc(syscurpos)
{
#ifdef USE_TERMCAP_DB
   static char cposbuf[100], * cpos = cposbuf;
#else
   static const char cpos[] = "\033[%d;%dH";
#endif

   if (argc != 0 && argc != 2)
       return BADARGS;

#ifdef USE_TERMCAP_DB
   if (!*cpos) {
      sethandles();
      cpos = tgetstr("cm", &cpos);
   }
#endif

   if (cpos) {
      int x, y;
      char * ex, *why;

      rxstrdup(ex, argv[1]);
      rxstrdup(why, argv[0]);
      x = atoi(ex)-1;
      y = atoi(why)-1;

#ifdef USE_TERMCAP_DB
      fputs(tgoto(cpos, x, y), stdout);
#else
      fprintf(stdout, cpos, y+1, x+1);
#endif
      fflush(stdout);
      strcpy(result->strptr, "0 0");
      result->strlength = 3;
   }
   else {
      strcpy(result->strptr, "0 0");
      result->strlength = 3;
   }

   return 0;
} 


/* syscurstate(state) */
rxfunc(syscurstate)
{
#ifdef USE_TERMCAP_DB
   static char css[100], *pcsson, *pcssoff;
#else
   /* these work for vt-series terminals */
   static char pcsson[] = "\033[?25h", pcssoff[] = "\033[?25l";
#endif
   char * onoff;

   checkparam(1, 1);

   rxstrdup(onoff, argv[0]);
   strupr(onoff);

#ifdef USE_TERMCAP_DB
   if (!*css) {
      char *pcss = css;
      sethandles();
      pcsson = tgetstr("ve", &pcss);
      pcssoff = tgetstr("vi", &pcss);
   }
#endif

   if (pcsson && pcssoff) {
       if (!strcasecmp(onoff, "OFF"))
          onoff = pcssoff;
       else
          onoff = pcsson;
   }

   fputs(onoff, stdout);
   fflush(stdout);

   return 0;
} 


/* sysgetkey([opt],[timeout]) */
rxfunc(sysgetkey)
{
   static struct termios oterm, nterm;
   static int init = 1;
   register rxbool doecho = true;
   fd_set readfds;
   struct timeval select_tv;
   struct timeval *pselect_tv = NULL;

   checkparam(0,2);

   if (argc > 0 && argv[0].strptr && (argv[0].strptr[0] == 'N' || argv[0].strptr[0] == 'n'))
      doecho = false;

   if (argc > 1 && argv[1].strptr) {
      select_tv.tv_sec = rxint(argv+1);
      select_tv.tv_usec = rxuint(argv+1);

      if (select_tv.tv_sec || select_tv.tv_usec) {
         pselect_tv = &select_tv;
      }
   }

   if (init) {
      init = 0;
      tcgetattr(0, &oterm);			/* save oterm state */
      nterm = oterm;				/* get base of nterm state */
      nterm.c_lflag &= ~(ICANON | ISIG | IEXTEN);
      nterm.c_iflag &= ~(ICRNL | INLCR | IXON | IXOFF | IXANY | BRKINT);
      nterm.c_cc[VMIN]  = 1;
      nterm.c_cc[VTIME] = 0;
   }

   if (!doecho)
       nterm.c_lflag &= ~(ECHO|ECHOE|ECHOK);
   else
       nterm.c_lflag |= ECHO|ECHOE|ECHOK;

   tcsetattr(0, TCSANOW, &nterm);			/* set mode */
   /*
    * Wait for specified timeout
    */
   FD_ZERO(&readfds);
   FD_SET(0, &readfds);
   if (select(1,&readfds,NULL,0,pselect_tv) > 0) {
      result->strlength = sprintf(result->strptr, "%c", getchar());
   }
   else
      result->strlength = 0;
   tcsetattr(0, TCSANOW, &oterm);			/* restore mode */

   return 0;
} 

/* systextscreenread(row,column, len) */
rxfunc(systextscreenread)
{
   what();
   return 0;
} 

/* systextscreensize() -- don't use termcap because the termcap
 * result comes from the database, not the screen  */
rxfunc(systextscreensize)
{
#  ifdef TIOCGWINSZ
   struct winsize ws;

   /* Get the terminal size */
   if (ioctl(0, TIOCGWINSZ, &ws) || 0 == ws.ws_row)
      return 80;

   result->strlength = sprintf(result->strptr, "%d %d", ws.ws_row, ws.ws_col);
#  else
   /* if the window-size ioctl is not avaiable, use the COLUMNS and LINES
    * environment variables. These are specified by POSIX, but some systems
    * don't set them correctly. */
   char * columns = getenv("COLUMNS"), * lines = getenv("LINES");

   if (columns && lines) {
      result->strlength = sprintf(result->strptr, "%s %s", lines, columns);
   }
   else {
      static const char commonsize[] = "24 80"; /* a common size... */
      result->strlength = sizeof(commonsize) - 1;
      memcpy(result->strptr, commonsize, sizeof(commonsize) - 1);
   }
#  endif

   return 0;
} 
