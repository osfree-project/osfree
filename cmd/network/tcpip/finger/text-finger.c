/* text-finger.c -- How to do textual GNU fingering. */

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
#include <config.h>
#include <pwd.h>
/* #include <general.h> */

/* Tell CONNECTION to send us the info on USER, and then print that
   info out. */
void
text_finger (user, connection)
  char *user;
  int connection;
{
  int out_fd, in_fd;
  FILE *out_stream, *in_stream;

  /* This buffer is used only to fragment the data; it is not a limit. */
  char line[256];
  out_fd = dup (connection);
  in_fd = dup (connection);

  if (!(in_stream = fdopen (in_fd, "r"))
      || !(out_stream = fdopen (out_fd, "w")))
    return;

  fprintf (out_stream, "%s\r\n", user);
  fflush (out_stream);

  while (fgets (line, sizeof(line), in_stream))
    {
      int n = strlen (line);

      if (n > 1 && line[n - 2] == '\r')
        {
          line[n - 2] = 0;

          fputs (line, stdout);
          putc ('\n', stdout);
        }
      else
        fputs (line, stdout);
    }
  fflush (stdout);
  close (in_fd);
  close (out_fd);
  fclose (in_stream);
  fclose (out_stream);
}



