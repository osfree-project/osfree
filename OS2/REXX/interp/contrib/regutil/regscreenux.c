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
 * $Header: /opt/cvs/Regina/regutil/regscreenux.c,v 1.11 2021/07/11 05:09:22 mark Exp $
 */
#include "regutil.h"
#ifdef USE_TERMCAP_DB
# ifdef HAVE_NCURSES_H
#  include <ncurses.h>
# endif
# if defined( HAVE_TERM_H )
#  include <term.h>
# elif defined( HAVE_TERMCAP_H )
#  include <termcap.h>
# endif
#endif
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

#if 0
#ifdef USE_TERMCAP_DB
# ifdef HAVE_TERM_H
#  include <sys/ioctl.h>
#  include <curses.h>
#  include <term.h>
# else
#  include <termcap.h>
# endif
#endif
#endif

#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

#ifdef HAVE_READLINE_READLINE_H
# include <readline/readline.h>
#endif
#ifdef HAVE_READLINE_HISTORY_H
# include <readline/history.h>
#endif

/* Grr BEOS has fd_set and socket() in socket.h */
#ifdef HAVE_SOCKET_H
# include <socket.h>
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
   static char clrbuf[100]="", * clr = clrbuf;

   if (!*clr) {
      sethandles();
      clr = tgetstr("cl", (char **)&clrbuf);
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
      cpos = tgetstr("cm", (char **)&cposbuf);
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
      sethandles();
      pcsson = tgetstr("ve", (char **)&css);
      pcssoff = tgetstr("vi", (char **)&css);
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
   char *echo;
   checkparam(0,2);

   /*
    * opt can be "N", "NO", or "NOECHO", "ECHO"
    * The last two to be consistent with ooRexx
    */
   if (argc > 0 && argv[0].strptr) {
      rxstrdup(echo, argv[0]);
      strupr(echo);
      if (strcmp(echo,"N") == 0 || strcmp(echo,"NO") == 0 || strcmp(echo,"NOECHO") == 0 )
         doecho = false;
      else if (strcmp(echo,"ECHO") == 0 )
         doecho = true;
      else
         return BADARGS;
   }

   if (argc > 1 && argv[1].strptr) {
      select_tv.tv_sec = rxint(argv+1);
      select_tv.tv_usec = rxuint(argv+1);

      if (select_tv.tv_sec || select_tv.tv_usec) {
         pselect_tv = &select_tv;
      }
   }

   if (init) {
      init = 0;
      tcgetattr(0, &oterm);                     /* save oterm state */
      nterm = oterm;                            /* get base of nterm state */
      nterm.c_lflag &= ~(ICANON | ISIG | IEXTEN);
      nterm.c_iflag &= ~(ICRNL | INLCR | IXON | IXOFF
#ifdef IXANY
                       | IXANY
#endif
#ifdef BRKINT
                       | BRKINT
#endif
                       );
      nterm.c_cc[VMIN]  = 1;
      nterm.c_cc[VTIME] = 0;
   }

   if (!doecho)
       nterm.c_lflag &= ~(ECHO|ECHOE|ECHOK);
   else
       nterm.c_lflag |= ECHO|ECHOE|ECHOK;

   tcsetattr(0, TCSANOW, &nterm);                       /* set mode */
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
   tcsetattr(0, TCSANOW, &oterm);                       /* restore mode */

   return 0;
}

rxfunc(sysgetline)
{
#ifdef HAVE_READLINE_HISTORY_H
   char *prompt = NULL;
   char *expansion;
   char *line;
   int rc;

   if (argc > 0 && argv[0].strptr)
   {
      prompt = argv[0].strptr;
   }
   line = readline( prompt );
   if ( line && line[0] )
   {
      rc = history_expand(line, &expansion);
      if (rc)
         fprintf (stderr, "%s\n", expansion);

      if (rc < 0 || rc == 2)
      {
         free (expansion);
      }
      else
      {
         add_history(expansion);
         strncpy(result->strptr, expansion, strlen(expansion));
         result->strlength = strlen( result->strptr );
         free(expansion);
      }
   }
   else
   {
      strcpy(result->strptr, "");
      result->strlength = 0;
   }
#else
   what();
#endif
   return 0;
}

rxfunc(sysgetlinehistory)
{
#ifdef HAVE_READLINE_HISTORY_H
   char *action;
   char *filename = NULL;
   register rxbool readhistory=false, inithistory=false;
   int rc=0;

   checkparam(2,2);

   if (argv[0].strptr)
   {
      filename = argv[0].strptr;
   }
   else
      return BADARGS;
   if ( argv[1].strptr )
   {
      rxstrdup(action, argv[1]);
      strupr(action);
      if (strcmp(action,"R") == 0 || strcmp(action,"READ") == 0 )
         readhistory = true;
      else if (strcmp(action,"W") == 0 || strcmp(action,"WRITE") == 0 )
         readhistory = false;
      else if (strcmp(action,"I") == 0 || strcmp(action,"INIT") == 0 )
         inithistory = true;
      else
         return BADARGS;
   }
   else
      return BADARGS;
   if ( inithistory )
   {
      using_history();
   }
   else if ( readhistory )
   {
      rc = read_history( filename );
   }
   else
   {
      rc = write_history( filename );
   }
   if (rc)
      result_one();
   else
      result_zero();
#else
   what();
#endif
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
   char * mycolumns = getenv("COLUMNS"), * mylines = getenv("LINES");

   if (mycolumns && mylines) {
      result->strlength = sprintf(result->strptr, "%s %s", mylines, mycolumns);
   }
   else {
      static const char commonsize[] = "24 80"; /* a common size... */
      result->strlength = sizeof(commonsize) - 1;
      memcpy(result->strptr, commonsize, sizeof(commonsize) - 1);
   }
#  endif

   return 0;
}
