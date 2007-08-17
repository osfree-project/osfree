/* finger.c -- The program that users directly interact with. */

/* Copyright (C) 1988, 1990, 1992 Free Software Foundation, Inc.

   This file is part of GNU Finger.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include <setjmp.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/file.h>

#include <general.h>
#include <tcp.h>
#include <error.h>
#ifndef __EMX__
#include <fingerpaths.h>
#include <face-finger.h>
#endif
#include <text-finger.h>
#include <netinet/in.h>

#include "getopt.h"
extern int tcp_to_service(char *service, char *address);
extern int default_error_handling (char *progname);

/* **************************************************************** */
/*                                                                  */
/*                            "finger"                              */
/*                                                                  */
/* **************************************************************** */

/* Our version of "finger". */

/* Possible values of the "options" to finger. */
#define INFO 0x02
#define VERS 0x04

void call_finger ();
extern char *baseprefix ();


static void
usage (argc, argv)
  int argc;
  char **argv;
{
  fprintf (stderr,
           "Usage: %s [-v] [--version] [-lim] [--info] [-f] [--faces]\n", baseprefix (*argv));
  fprintf (stderr, "       [-sb] [--brief] [-P port#] [--port port#] [-h] [--help]\n");
  fprintf (stderr, "       [user | user@host ...]\n");
  exit (1);
}


static void
help ()
{
  fputs ("This is (not quite) GNU Finger. Choose among the following options:\n\n", stderr);
  fputs ("-v, \t\tdisplay version number\n", stderr);
  fputs ("-i, -l, -m,\tdisplay full user information\n", stderr);
  fputs ("-b, -s, \topposite of -i; only display login records\n", stderr);
  fputs ("-P,    #p\tconnect to finger daemon using port or service #p\n", stderr);
  fputs ("-h, \t\tdisplay this message\n", stderr);
  fputc ('\n', stderr);

  exit (1);
}


main (argc, argv)
  int argc;
  char **argv;
{
  int arg_index, opt_index = 0;
  long options;
  char *arg, *port;
  int optc;

#ifdef ERROR_HANDLER
default_error_handling (argv[0]);
#endif
  /* Parse the arguments passed on the command line. */

#if defined (INFO_IS_DEFAULT)
  options = INFO;
#else
  options = 0;
#endif

  arg_index = 1;
  port = NULL;

  while ((optc = getopt (argc, argv, "vfilmP:bsh")) > 0)
    switch (optc)
      {
      case 'i':
      case 'l':
      case 'm':

        options |= INFO;
        break;

      case 'b':
      case 's':

        options &= ~INFO;
        break;

      case 'v':

        options |= VERS;
        break;

      case 'P':

        port = optarg;
        break;

      case 'h':

        help ();

      default:

        usage (argc, argv);
      }

  if (options & VERS)
    printf ("(reduced) GNU Finger client version %s.\n", VERSION_STRING);

#ifdef BASENAME_IS_SERVICE
  if (!port)
    port = baseprefix (argv[0]);
#endif

  if (optind >= argc)
    call_finger ("", options, port);
  else
    for (arg_index = optind; arg_index < argc; arg_index++)
      call_finger (argv[arg_index], options, port, arg_index,argc);

  exit (1);
}

/* Call the finger daemon to actually do the fingering. */
void
call_finger (arg, options, portnum, arg_ind, argcount)
  char *arg, *portnum;
  long options;
  int arg_ind, argcount;
{
  char *t;
  char *username, *hostname;
  char *localhost;
  struct hostent *host = NULL;
  int connection, info;
  long addr;
   int suppress_hostname = 0;
  int n_flag=0;

  username = savestring (arg);
  info = options & INFO;


  hostname = NULL;
      for (t = username; *t && *t != '@'; t++);

      if (*t)
        hostname = t + 1;

      *t = '\0';

  if (!hostname)
    {
      if (!(localhost = xgethostname ()))
        localhost = "localhost";

      hostname = localhost;
      suppress_hostname = 1;
    }

  if (digit (*hostname))
    {
      addr = (long)inet_addr (hostname);
      host = gethostbyaddr ((const char *) &addr, 4, AF_INET);

      if (!host)
        {
          host = (struct hostent *) xmalloc (sizeof (struct hostent));
         n_flag=1;
          host->h_name = hostname;
        }
    }
  else
    {
      host = gethostbyname (hostname);

      if (!host)
        handle_error (FATAL, "%s: unknown host", hostname);

      bcopy (host->h_addr, &addr, 4);
    }

  if (!suppress_hostname)
    {
      printf ("[%s]", host->h_name);
      fflush (stdout);
    }

  connection = tcp_to_service (portnum ? portnum : "finger", (char *) &addr);

  if (connection < 0)
    {
      printf ("\n");
      fflush (stdout);
      file_error (FATAL, host->h_name);
    }

  if (!suppress_hostname)
    {
      printf ("\n");
      fflush (stdout);
    }

  /* Okay, connection established.  Now actually do the fingering. */
   if (info && *username)
        {
          char *new_username = (char *) xmalloc (4 + strlen (username));
          strcpy (new_username, "/W ");
          strcat (new_username, username);
          free (username);
          username = new_username;

         if (arg_ind == argcount) free(new_username);
        }

   text_finger (username, connection);

  close (connection);
  free (username);

if (host && n_flag)
    free (host);
}
