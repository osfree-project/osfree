/* getline.c -- Replacement for GNU C library function getline ()

   Copyright (C) 1992 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */


#include <stdio.h>

#define MAX_CANON 64

/* Read up to (and including) a newline from STREAM into *LINEPTR
   (and NUL-terminate it). *LINEPTR is a pointer returned from malloc (or
   NULL), pointing to *N characters of space.  It is realloc'd as
   necessary.  Returns the number of characters read (not including the
   null terminator), or -1 on error or EOF.  */

int
getline (lineptr, n, stream)
  char **lineptr;
  unsigned int *n;
  FILE *stream;
{
  int nchars_avail;
  char *read_pos;
  extern char *malloc (), *realloc ();


  if (!lineptr || !n || !stream)
    return -1;

  nchars_avail = *n;

  if (!*lineptr)
    {
      if (!(*lineptr = malloc (MAX_CANON)))
	return -1;

      *n = nchars_avail = MAX_CANON;
    }

  read_pos = *lineptr;

  for (;;)
    {
      register char c = getc (stream);

      /* We always want at least one char left in buffer since we
	 always (unless we get an error while reading the first char)
	 NUL-terminate the line buffer. */

      if (nchars_avail < 1)
	{
	  if (*n > MAX_CANON)
	    {
	      nchars_avail = *n;
	      *n *= 2;
	    }
	  else
	    {
	      nchars_avail = MAX_CANON;
	      *n += MAX_CANON;
	    }

	  *lineptr = realloc (*lineptr, *n);
	  read_pos = *lineptr + (*n - nchars_avail);
	}

      /* EOF or error */
      if (feof (stream) || ferror (stream))

	/* Return partial line, if any */
	if (read_pos == *lineptr)
	  return -1;
	else
	  break;

      *read_pos++ = c;
      nchars_avail--;

      /* Return line if NL */
      if (c == '\n')
	break;
    }

  /* Done - NUL terminate and return number of chars read */
  *read_pos = '\0';
  return (*n - nchars_avail);
}
