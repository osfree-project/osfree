/* error.c -- Simple error handling. */

/* Copyright (C) 1988, 1990, 1992  Free Software Foundation, Inc.

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
#ifdef __EMX__
#include <stdlib.h>
#endif
#include <setjmp.h>
#include <config.h>
#include <general.h>
#include <error.h>

/* These can be filled in `manually' by callers, but the easiest way
   is to call default_error_handling (argv[0]). */
char *progname = "finger:-(";
jmp_buf top_level;

/* **************************************************************** */
/*                                                                  */
/*                      Error Handler                               */
/*                                                                  */
/* **************************************************************** */

default_error_handling (program_name)
     char *program_name;
{
  if ((char *)rindex (program_name, '/') != (char *)NULL)
    {
      program_name = (char *)rindex (program_name, '/');
      program_name++;
    }

  progname = savestring (program_name);
  strcat(progname,":");
  if (setjmp (top_level))
    exit (1);
}

/* Hack to handle previous bad setjmp (). */
longjmperror ()
{
  exit (1);
}

/* Handle some error. */
void
handle_error (severity, format, arg1, arg2, arg3)
     int severity;
     char *format, *arg1, *arg2, *arg3;
{
  fprintf (stderr, "%s  ", progname);
  fprintf (stderr, format, arg1, arg2, arg3);
  fprintf (stderr, "\n");

  switch (severity)
    {
    case FATAL:
      exit (1);

    case ERROR:
      longjmp (top_level, 0);

    case WARNING:
      break;

    default:
      fprintf (stderr, "\
handle_error () called with bad severity (%d).\n\
Who knows what else is wrong with the code?  Stopping.\n", severity);
      exit (2);
    }
}

/* Handle a file error.  You pass severity and filename. If FILENAME is null,
   then don't print it. */
void
file_error (severity, filename)
     int severity;
     char *filename;
{
  extern int errno, sys_nerr;
#ifndef __EMX__
  extern char *sys_errlist[];
#endif
  char *error_text;

  if (errno) {
    if (errno < sys_nerr)
      error_text = sys_errlist[errno];
    else
      error_text = "(large errno?)";
  } else {
    error_text = "(Hmm, no error?)";
  }

  if (filename == (char *)NULL)
    handle_error (severity, "%s", error_text);
  else
    handle_error (severity, "%s: %s", filename, error_text);
}

warning (format, arg1, arg2, arg3)
     char *format, *arg1, *arg2, *arg3;
{
  handle_error (WARNING, format, arg1, arg2, arg3);
}
