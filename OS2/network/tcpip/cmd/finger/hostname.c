/* hostname.c -- Wrapper for gethostname(). */

/* Copyright (C) 1992  Free Software Foundation, Inc.

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
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <general.h>


/* Return host name. Assuming the host name doesn't change between
   calls, we only call gethostname() once. */
char *
xgethostname ()
{
  static char *hostname = NULL;

  /* This is just an initial guess */
  static int hostname_size = 128;

  /* Deja Vu  */
  if (hostname)
    return hostname;


  /* Obtain host name */
  hostname = xmalloc (hostname_size);
  for (;;)
    {
      hostname[hostname_size - 1] = 0;

      if (gethostname (hostname, hostname_size) < 0)
        {
          free (hostname);
          hostname = NULL;
          return NULL;
        }

      if (!hostname[hostname_size - 1])
        return hostname;

      hostname_size *= 2;
      hostname = xrealloc (hostname, hostname_size);
    }
}
