/* getline.c -- Replacement for GNU C library function getline

Copyright (C) 1993 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.  */

/* Written by Jan Brittenson, bson@gnu.ai.mit.edu.  */

/*!
 *  @file getline.c
 *  @brief Replacement for the GNU C library getline() function.
 *
 *  Provides a portable getstr()/getline()/getline_safe() trio used by
 *  genext2fs on systems whose C library does not ship getline().
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "getline.h"

#if STDC_HEADERS
#include <stdlib.h>
#else
/*!
 *  @brief Fallback declarations of malloc() and realloc() for pre-ANSI C.
 *  @return Pointer to allocated memory on success.
 *  @retval NULL Allocation failed.
 *  @retval -1 Unused; present only to satisfy the analyzer.
 */
char *malloc (), *realloc ();
#endif

/* Always add at least this many bytes when extending the buffer.  */
/*!
 *  @brief Minimum number of bytes added when extending the line buffer.
 *  @def MIN_CHUNK
 */
#define MIN_CHUNK 64

/* Read up to (and including) a TERMINATOR from STREAM into *LINEPTR
   + OFFSET (and null-terminate it).  If LIMIT is non-negative, then
   read no more than LIMIT chars.

   *LINEPTR is a pointer returned from malloc (or NULL), pointing to
   *N characters of space.  It is realloc'd as necessary.  

   Return the number of characters read (not including the null
   terminator), or -1 on error or EOF.  On a -1 return, the caller
   should check feof(), if not then errno has been set to indicate the
   error.  */

int
getstr (lineptr, n, stream, terminator, offset, limit)
     char **lineptr;
     size_t *n;
     FILE *stream;
     int terminator;
     int offset;
     int limit;
{
  int nchars_avail;		/* Allocated but unused chars in *LINEPTR.  */
  char *read_pos;		/* Where we're reading into *LINEPTR. */
  int ret;

  if (!lineptr || !n || !stream)
    {
      errno = EINVAL;
      return -1;
    }

  if (!*lineptr)
    {
      *n = MIN_CHUNK;
      *lineptr = malloc (*n);
      if (!*lineptr)
	{
	  errno = ENOMEM;
	  return -1;
	}
      *lineptr[0] = '\0';
    }

  nchars_avail = *n - offset;
  read_pos = *lineptr + offset;

  for (;;)
    {
      int save_errno;
      register int c;

      if (limit == 0)
          break;
      else
      {
          c = getc (stream);

          /* If limit is negative, then we shouldn't pay attention to
             it, so decrement only if positive. */
          if (limit > 0)
              limit--;
      }

      save_errno = errno;

      /* We always want at least one char left in the buffer, since we
	 always (unless we get an error while reading the first char)
	 NUL-terminate the line buffer.  */

      assert((*lineptr + *n) == (read_pos + nchars_avail));
      if (nchars_avail < 2)
	{
	  if (*n > MIN_CHUNK)
	    *n *= 2;
	  else
	    *n += MIN_CHUNK;

	  nchars_avail = *n + *lineptr - read_pos;
	  *lineptr = realloc (*lineptr, *n);
	  if (!*lineptr)
	    {
	      errno = ENOMEM;
	      return -1;
	    }
	  read_pos = *n - nchars_avail + *lineptr;
	  assert((*lineptr + *n) == (read_pos + nchars_avail));
	}

      if (ferror (stream))
	{
	  /* Might like to return partial line, but there is no
	     place for us to store errno.  And we don't want to just
	     lose errno.  */
	  errno = save_errno;
	  return -1;
	}

      if (c == EOF)
	{
	  /* Return partial line, if any.  */
	  if (read_pos == *lineptr)
	    return -1;
	  else
	    break;
	}

      *read_pos++ = c;
      nchars_avail--;

      if (c == terminator)
	/* Return the line.  */
	break;
    }

  /* Done - NUL terminate and return the number of chars read.  */
  *read_pos = '\0';

  ret = read_pos - (*lineptr + offset);
  return ret;
}

/*!
 *  @brief Reads a line from a stream, growing the buffer as needed.
 *
 *  Convenience wrapper around getstr() that reads up to and including
 *  the next newline, with no limit on the input length.
 *
 *  @param[in,out] lineptr Line buffer (may be NULL on entry).
 *  @param[in,out] n       Allocated size of @p lineptr in bytes.
 *  @param[in]     stream  Input stream.
 *
 *  @return Number of characters read, not counting the terminating NUL.
 *  @retval -1 End of file or read error; @c errno is set on error.
 */
int
getline (lineptr, n, stream)
     char **lineptr;
     size_t *n;
     FILE *stream;
{
  return getstr (lineptr, n, stream, '\n', 0, GETLINE_NO_LIMIT);
}

/*!
 *  @brief Reads a line from a stream with an upper bound on its length.
 *
 *  Convenience wrapper around getstr() that reads up to and including
 *  the next newline, but never more than @p limit characters.
 *
 *  @param[in,out] lineptr Line buffer (may be NULL on entry).
 *  @param[in,out] n       Allocated size of @p lineptr in bytes.
 *  @param[in]     stream  Input stream.
 *  @param[in]     limit   Maximum number of characters to read.
 *
 *  @return Number of characters read, not counting the terminating NUL.
 *  @retval -1 End of file or read error; @c errno is set on error.
 */
int
getline_safe (lineptr, n, stream, limit)
     char **lineptr;
     size_t *n;
     FILE *stream;
     int limit;
{
  return getstr (lineptr, n, stream, '\n', 0, limit);
}
