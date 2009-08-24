/* dynstr.c -- dynamic string functions

  AUTHOR: Gregory Pietsch

  DESCRIPTION:

  This file contains dynamic string functions for an 
  edlin-style line editor.

  COPYRIGHT NOTICE AND DISCLAIMER:

  Copyright (C) 2003 Gregory Pietsch

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
*/

/* includes */

#include "config.h"
#if defined(STDC_HEADERS) || defined(HAVE_LIMITS_H)
#include <limits.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <stdio.h>
#if defined(STDC_HEADERS) || defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif
#if defined(STDC_HEADERS) || defined(HAVE_STRING_H)
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include "defines.h"
#include "dynstr.h"
#include "msgs.h"

/* macros */

#define MIN_SIZE 31

/* typedefs */

/* functions */

/* This file needs memchr(), memmove(), and memset().  If these functions are 
   not around, I have to provide them.  */

#ifndef HAVE_MEMCHR

#ifndef _OPTIMIZED_FOR_SIZE
 /* Nonzero if either X or Y is not aligned on an "unsigned long" boundary.  */
#ifdef _ALIGN
#define UNALIGNED(X) \
  ((unsigned long)X&(sizeof(unsigned long)-1))
#else
#define UNALIGNED(X) (0)
#endif

 /* How many bytes are copied each interation of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof(unsigned long))

 /* Threshhold for punting to the bytewise iterator.  */
#define TOO_SMALL(len) ((len)<LITTLEBLOCKSIZE)

 /* Null character detection.  */
#if ULONG_MAX == 0xFFFFFFFFUL
#define DETECTNULL(X) (((X)-0x01010101UL)&~(X)&0x80808080UL)
#elif ULONG_MAX == 0xFFFFFFFFFFFFFFFFUL
#define DETECTNULL(X) (((X)-0x0101010101010101UL)&~(X)&0x8080808080808080UL)
#else
#error unsigned long is not 32 or 64 bits wide
#endif

#if UCHAR_MAX != 0xFF
#error unsigned char is not 8 bits wide
#endif

#endif /* _OPTIMIZED_FOR_SIZE */

static void *
memchr (const void *s, int c, size_t n)
{
  const unsigned char *us;
  unsigned char uc = (unsigned char) c;

#ifndef _OPTIMIZED_FOR_SIZE
  unsigned long *psrc;
  size_t i;
  unsigned long mask = 0, buffer = 0;
#endif

  us = s;
#ifndef _OPTIMIZED_FOR_SIZE
  /* If the size is small, or s is unaligned, punt into the bytewise loop.
     This should be rare.
   */
  if (!TOO_SMALL (n) && !UNALIGNED (s))
    {
      psrc = (unsigned long *) s;

      /* The fast code reads the data one word at a time and only performs
         the bytewise search on word-sized segments if they contain the
         search character, which is detected by XORing the word-sized
         segment with a word-sized block of the search character and then
         detecting the presence of a null character in the result.
       */
      for (i = 0; i < LITTLEBLOCKSIZE; i++)
        mask = (mask << CHAR_BIT) + ((unsigned char) uc & ~(~0 << CHAR_BIT));

      /* Check a block at a time if possible. */
      while (n >= LITTLEBLOCKSIZE)
        {
          buffer = *psrc ^ mask;
          if (DETECTNULL (buffer))
            break;              /* found character, so go byte by byte from here */
          n -= LITTLEBLOCKSIZE;
          psrc++;
        }

      /* Pick up any residual with a bytewise iterator.  */
      us = (unsigned char *) psrc;
    }
#endif
  /* The normal bytewise loop.  */

  while (n--)
    {
      if (*us == uc)
        return (void *) us;
      us++;
    }
  return 0;
}

#ifndef _OPTIMIZED_FOR_SIZE
#undef UNALIGNED
#undef LITTLEBLOCKSIZE
#undef TOO_SMALL
#undef DETECTNULL
#endif /* _OPTIMIZED_FOR_SIZE */

#endif /* HAVE_MEMCHR */

#ifndef HAVE_MEMMOVE

#ifndef _OPTIMIZED_FOR_SIZE

 /* Nonzero if either X or Y is not aligned on an "unsigned long" boundary.  */
#ifdef _ALIGN
#define UNALIGNED(X,Y) \
  (((unsigned long)X&(sizeof(unsigned long)-1))\
  |((unsigned long)Y&(sizeof(unsigned long)-1)))
#else
#define UNALIGNED(X,Y) (0)
#endif

 /* How many bytes are copied each interation of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof(unsigned long))

 /* How many bytes are copied each interation of the 4X unrolled loop.  */
#define BIGBLOCKSIZE (sizeof(unsigned long)<<2)

 /* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(len) ((len)<BIGBLOCKSIZE)

#endif /* _OPTIMIZED_FOR_SIZE */

static void *
memmove (void *s1, const void *s2, size_t n)
{
  unsigned char *us1 = s1;
  const unsigned char *us2 = s2;

#ifndef _OPTIMIZED_FOR_SIZE
  unsigned long *pdst, *psrc;
#endif

  if (us2 < us1 && us1 < us2 + n)
    {
      /* Have to copy backwards.  */
      us1 += n;
      us2 += n;
      while (n--)
        *--us1 = *--us2;
      return s1;
    }
#ifndef _OPTIMIZED_FOR_SIZE
  /* If the size is small, or either s1 or s2 is unaligned, punt into the
     byte copy loop.  This should be rare.  */
  if (!TOO_SMALL (n) && !UNALIGNED (s2, s1))
    {
      pdst = (unsigned long *) s1;
      psrc = (unsigned long *) s2;

      /* Copy a big block at a time if possible. */
      while (n >= BIGBLOCKSIZE)
        {
          *pdst++ = *psrc++;
          *pdst++ = *psrc++;
          *pdst++ = *psrc++;
          *pdst++ = *psrc++;
          n -= BIGBLOCKSIZE;
        }

      /* Copy a little block at a time if possible. */
      while (n >= LITTLEBLOCKSIZE)
        {
          *pdst++ = *psrc++;
          n -= LITTLEBLOCKSIZE;
        }

      /* Pick up any residual with a byte copier.  */
      us1 = (unsigned char *) pdst;
      us2 = (unsigned char *) psrc;
    }
#endif
  /* The normal byte-copy loop.  */

  while (n--)
    *us1++ = *us2++;
  return s1;
}

#ifndef _OPTIMIZED_FOR_SIZE
#undef UNALIGNED
#undef LITTLEBLOCKSIZE
#undef BIGBLOCKSIZE
#undef TOO_SMALL
#endif /* _OPTIMIZED_FOR_SIZE */

#endif /* HAVE_MEMMOVE */

#ifndef HAVE_MEMSET

#ifndef _OPTIMIZED_FOR_SIZE

 /* Nonzero if X is not aligned on an "unsigned long" boundary.  */
#ifdef _ALIGN
#define UNALIGNED(X) ((unsigned long)X&(sizeof(unsigned long)-1))
#else
#define UNALIGNED(X) (0)
#endif

 /* How many bytes are copied each interation of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof(unsigned long))

 /* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(len) ((len)<LITTLEBLOCKSIZE)

#endif /* _OPTIMIZED_FOR_SIZE */

static void *
memset (void *s, int c, size_t n)
{
  unsigned char *us = s;
  unsigned char uc = (unsigned char) c;

#ifndef _OPTIMIZED_FOR_SIZE
  unsigned long *ps;
  unsigned long mask = 0;
  size_t i;

  /* If the size is small, or s is unaligned, punt into the
     byte copy loop.  This should be rare.  */
  if (!TOO_SMALL (n) && !UNALIGNED (s))
    {
      ps = (unsigned long *) s;
      /* Store uc into mask at each location.  */
      for (i = 0; i < LITTLEBLOCKSIZE; i++)
        mask =
          ((mask << CHAR_BIT) + ((unsigned char) uc & ~(~0 << CHAR_BIT)));

      /* Copy a 4X block at a time if possible. */
      while (n >= LITTLEBLOCKSIZE * 4)
        {
          *ps++ = mask;
          *ps++ = mask;
          *ps++ = mask;
          *ps++ = mask;
          n -= LITTLEBLOCKSIZE * 4;
        }

      /* Copy a block at a time if possible. */
      while (n >= LITTLEBLOCKSIZE)
        {
          *ps++ = mask;
          n -= LITTLEBLOCKSIZE;
        }

      /* Pick up any residual with a byte copier.  */
      us = (unsigned char *) ps;
    }
#endif
  /* The normal byte-copy loop.  */

  while (n--)
    *us++ = uc;
  return s;
}

#ifndef _OPTIMIZED_FOR_SIZE
#undef UNALIGNED
#undef LITTLEBLOCKSIZE
#undef TOO_SMALL
#endif /* _OPTIMIZED_FOR_SIZE */

#endif /* HAVE_MEMSET */

/* DStidy: Tidy up the fields in the STRING_T.
   This function is called right after a string is allocated and
   also right before it is freed to initialize the contents to 0.
   constructed - true if the string was constructed, false if not.  */
static void
DStidy (STRING_T * this, int constructed)
{
  if (constructed && this->ptr)
    free (this->ptr);
  this->ptr = 0;
  this->len = 0;
  this->res = 0;
}

/* DSgrow: Adjusts the allocated memory inside this->ptr to n if
   n is bigger than the amount that has already been allocated
   (in this->res).  If trim is true, do a reallocation even if
   n is less than the original size.  */
static int
DSgrow (STRING_T * this, size_t n, int trim)
{
  size_t osize = this->ptr == 0 ? 0 : this->res;
  size_t size;
  char *s;

  if (n == 0)
    {
      if (trim && MIN_SIZE < osize)
        DStidy (this, 1);
      else if (this->ptr)
        this->ptr[this->len = 0] = '\0';
      return 0;
    }
  else if (n == osize || (n < osize && !trim))
    return 1;
  else
    {
      size = this->ptr == 0 && n < this->res ? this->res : n;
      if ((size |= MIN_SIZE) == NPOS)
        --size;
      if ((s = (char *) realloc (this->ptr, size + 1)) == 0
          && (s = (char *) realloc (this->ptr, (size = n) + 1)) == 0)
        Nomemory ();
      this->ptr = s;
      this->res = size;
      return 1;
    }
}

/* DSxlen: Output a string too long error and abort.  */
static void
DSxlen (void)
{
  fprintf (stderr, "%s\n", G00031);
  abort ();
}

/* DSxran: Output an invalid string position error and abort.  */
static void
DSxran (void)
{
  fprintf (stderr, "%s\n", G00032);
  abort ();
}

/* exported functions */

/* DSctor: Construct a string.  The this pointer should point to newly
   allocated memory -- malloc, realloc, or calloc, it doesn't matter.  */
void
DSctor (STRING_T * this)
{
  DStidy (this, 0);
}

/* DSctor_with_size: Construct a string with a default size.  If c is
   default_size, initialize the this string to n null characters.  */
void
DSctor_with_size (STRING_T * this, size_t n, capacity c)
{
  DStidy (this, 0);
  this->res = n;
  if (c == default_size)
    DSassignchar (this, '\0', n);
}

/* DScopy_ctor: Construct a string that is a copy of another string.  */
void
DScopy_ctor (STRING_T * this, STRING_T * x)
{
  DStidy (this, 0);
  DSassign (this, x, 0, NPOS);
}

/* DSdtor: Destroy a string. */
void
DSdtor (STRING_T * this)
{
  DStidy (this, 1);
}

/* DScreate: Allocate memory for a string, then call the default constructor
   (DSctor). */
STRING_T *
DScreate (void)
{
  STRING_T *this = malloc (sizeof (STRING_T));

  if (this == 0)
    Nomemory ();
  DSctor (this);
  return this;
}

/* DScreate_copy: Allocate memory for a string, then call the copy constructor
   (DSctor). */
STRING_T *
DScreate_copy (STRING_T *x)
{
  STRING_T *this = malloc (sizeof (STRING_T));

  if (this == 0)
    Nomemory ();
  DScopy_ctor (this, x);
  return this;
}

/* DScreate_with_size: Allocate memory for a string, then call the constructor
   (DSctor_with_size). */
STRING_T *
DScreate_with_size (size_t n, capacity c)
{
  STRING_T *this = malloc (sizeof (STRING_T));

  if (this == 0)
    Nomemory ();
  DSctor_with_size (this, n, c);
  return this;
}

/* DSdestroy: Destroy a string, then free it.  */
void
DSdestroy (STRING_T * this)
{
  DSdtor (this);
  free (this);
}

/* DSappendchar: Append nr c characters to the end of this.  */
STRING_T *
DSappendchar (STRING_T * this, int c, size_t nr)
{
  size_t n;

  if (NPOS - this->len <= nr)
    DSxlen ();
  if (0 < nr && DSgrow (this, n = this->len + nr, 0))
    {
      memset (this->ptr + this->len, c, nr);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSappendcstr: Append a C-string, s, to the end of this.
   The ns parameter specifies the number of characters from s to
   append.  Set ns to NPOS to append the whole string.  */
STRING_T *
DSappendcstr (STRING_T * this, char *s, size_t ns)
{
  size_t n;

  if (ns == NPOS)
    ns = strlen (s);
  if (NPOS - this->len <= ns)
    DSxlen ();
  if (0 < ns && DSgrow (this, n = this->len + ns, 0))
    {
      memcpy (this->ptr + this->len, s, ns);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSappend: Append a substring of a STRING_T, str, to the end of this.
   pos indicates the position within str to start taking characters to
   append; ns indicates the number of characters to take (use NPOS to
   take the whole string).  */
STRING_T *
DSappend (STRING_T * this, STRING_T * str, size_t pos, size_t ns)
{
  size_t n;

  if (DSlength (str) < pos)
    DSxran ();
  n = DSlength (str) - pos;
  if (n < ns)
    ns = n;
  if (NPOS - this->len <= ns)
    DSxlen ();
  if (0 < ns && DSgrow (this, n = this->len + ns, 0))
    {
      memcpy (this->ptr + this->len, DScstr (str) + pos, ns);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSassignchar: Make the this->ptr string point to n repetitions of
   character c.  */
STRING_T *
DSassignchar (STRING_T * this, int c, size_t n)
{
  if (n == NPOS)
    DSxlen ();
  if (DSgrow (this, n, 1))
    {
      memset (this->ptr, c, n);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}


/* DSassigncstr: Make the this->ptr string point to a string
   comprised of the first n characters of s.  Use NPOS for n if you
   want the whole string.  */
STRING_T *
DSassigncstr (STRING_T * this, char *s, size_t n)
{
  if (n == NPOS)
    n = strlen (s);
  if (DSgrow (this, n, 1))
    {
      memcpy (this->ptr, s, n);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSassign: Assign a substring (possible the whole string) of str
   to this.  The pos parameter specifies the position within str
   to start; the ns parameter specifies the number of characters
   to take (use NPOS for the whole string).  NOTE: this and str
   can be the same string; it does what you expect.  */
STRING_T *
DSassign (STRING_T * this, STRING_T * str, size_t pos, size_t ns)
{
  size_t n;

  if (DSlength (str) < pos)
    DSxran ();
  n = DSlength (str) - pos;
  if (ns < n)
    n = ns;
  if (this == str)
    {
      DSremove (this, pos + n, NPOS);
      DSremove (this, 0, pos);
    }
  else if (DSgrow (this, n, 1))
    {
      memcpy (this->ptr, DScstr (str) + pos, n);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSinsertchar: Insert nr characters with value c at position p0
   within this.  Return the resulting string.  */
STRING_T *
DSinsertchar (STRING_T * this, size_t p0, int c, size_t nr)
{
  size_t n;

  if (this->len < p0)
    DSxran ();
  if (NPOS - this->len <= nr)
    DSxlen ();
  if (0 < nr && DSgrow (this, n = this->len + nr, 0))
    {
      memmove (this->ptr + (p0 + nr), this->ptr + p0, this->len - p0);
      memset (this->ptr + p0, c, nr);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSinsertcstr: Insert ns characters from string pointed to by s
   into the string pointed to by this at position p0. As usual, use
   NPOS for the ns parameter if you want to insert the whole string.  */
STRING_T *
DSinsertcstr (STRING_T * this, size_t p0, char *s, size_t ns)
{
  size_t n;

  if (this->len < p0)
    DSxran ();
  if (ns == NPOS)
    ns = strlen (s);
  if (NPOS - this->len <= ns)
    DSxlen ();
  if (0 < ns && DSgrow (this, n = this->len + ns, 0))
    {
      memmove (this->ptr + (p0 + ns), this->ptr + p0, this->len - p0);
      memcpy (this->ptr + p0, s, ns);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSinsert: Insert a substring of a STRING_T starting at position pos with
   length ns into a STRING_T this at position p0.  */
STRING_T *
DSinsert (STRING_T * this, size_t p0, STRING_T * str, size_t pos, size_t ns)
{
  size_t n;

  if (this->len < p0 || DSlength (str) < pos)
    DSxran ();
  n = DSlength (str) - pos;
  if (n < ns)
    ns = n;
  if (NPOS - this->len <= ns)
    DSxlen ();
  if (0 < ns && DSgrow (this, n = this->len + ns, 0))
    {
      memmove (this->ptr + (p0 + ns), this->ptr + p0, this->len - p0);
      memcpy (this->ptr + p0, DScstr (str) + pos, ns);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSremove: Remove nr characters from the string this from position p0.  */
STRING_T *
DSremove (STRING_T * this, size_t p0, size_t nr)
{
  size_t n;

  if (this->len < p0)
    DSxran ();
  if (this->len - p0 < nr)
    nr = this->len - p0;
  if (0 < nr)
    {
      memmove (this->ptr + p0, this->ptr + (p0 + nr), this->len - p0 - nr);
      n = this->len - nr;
      if (DSgrow (this, n, 0))
        this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSreplacechar: Replace the substring within string this at position p0
   for length n0 with nr repititions of character c.  */
STRING_T *
DSreplacechar (STRING_T * this, size_t p0, size_t n0, int c, size_t nr)
{
  size_t n, nm;

  if (this->len < p0)
    DSxran ();
  if (this->len - p0 < n0)
    n0 = this->len - p0;
  if (NPOS - nr <= this->len - n0)
    DSxlen ();
  nm = this->len - n0 - p0;
  if (nr < n0)
    memmove (this->ptr + (p0 + nr), this->ptr + (p0 + n0), nm);
  if ((0 < nr || 0 < n0) && DSgrow (this, n = this->len + nr - n0, 0))
    {
      if (n0 < nr)
        memmove (this->ptr + (p0 + nr), this->ptr + (p0 + n0), nm);
      memset (this->ptr + p0, c, nr);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSreplacecstr: Replace the substring within string this at position p0
   for length n0 with ns characters coming from the contents of the C 
   string s. If ns is NPOS, use the whole string s.  */
STRING_T *
DSreplacecstr (STRING_T * this, size_t p0, size_t n0, char *s, size_t ns)
{
  size_t n, nm;

  if (this->len < p0)
    DSxran ();
  if (ns == NPOS)
    ns = strlen (s);
  if (NPOS - ns <= this->len - n0)
    DSxlen ();
  nm = this->len - n0 - p0;
  if (ns < n0)
    memmove (this->ptr + (p0 + ns), this->ptr + (p0 + n0), nm);
  if ((0 < ns || 0 < n0) && DSgrow (this, n = this->len + ns - n0, 0))
    {
      if (n0 < ns)
        memmove (this->ptr + (p0 + ns), this->ptr + (p0 + n0), nm);
      memcpy (this->ptr + p0, s, ns);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSreplace: Replace the substring of string this at position p0
   with length n0 with ns characters coming from the string str
   starting at position pos.  If ns is NPOS, use the whole string.  */
STRING_T *
DSreplace (STRING_T * this, size_t p0, size_t n0, STRING_T * str,
           size_t pos, size_t ns)
{
  size_t n, nm;

  if (this->len < p0 || DSlength (str) < pos)
    DSxran ();
  n = DSlength (str) - pos;
  if (n < ns)
    ns = n;
  if (NPOS - ns <= this->len - n0)
    DSxlen ();
  nm = this->len - n0 - p0;
  if (ns < n0)
    memmove (this->ptr + (p0 + ns), this->ptr + (p0 + n0), nm);
  if ((0 < ns || 0 < n0) && DSgrow (this, n = this->len + ns - n0, 0))
    {
      if (n0 < ns)
        memmove (this->ptr + (p0 + ns), this->ptr + (p0 + n0), nm);
      memcpy (this->ptr + p0, DScstr (str) + pos, ns);
      this->ptr[this->len = n] = '\0';
    }
  return this;
}

/* DSget_at: Get the character at position p0 in string this.  */
int
DSget_at (STRING_T * this, size_t p0)
{
  if (this->len <= p0)
    DSxran ();
  return this->ptr[p0];
}

/* DSput_at: Put a character c into string this at position p0.  The
   p0 parameter could be at the position numbered by the length of the
   string, in which case the character is just appended.  */
void
DSput_at (STRING_T * this, size_t p0, int c)
{
  if (this->len < p0)
    DSxran ();
  else if (this->len == p0)
    DSappendchar (this, c, 1);
  else
    this->ptr[p0] = c;
}

/* DScopy: Copy the contents of this into the area pointed to by s.
   The n parameter is the number of characters to put; p0 is the
   position within this to start copying from.  */
size_t
DScopy (STRING_T * this, char *s, size_t n, size_t p0)
{
  if (this->len < p0)
    DSxran ();
  if (this->len - p0 < n)
    n = this->len - p0;
  memcpy (s, this->ptr + p0, n);
  return n;
}

/* DSfind: Find the position within this (starting at position p0) where the
   substring equal to the first n characters pointed to by s is.  (This is 
   similar to the strstr() function.)  Return NPOS on failure.  */
size_t
DSfind (STRING_T * this, char *s, size_t p0, size_t n)
{
  size_t nmax;
  char *t, *u;

  if (n == 0 || (n == NPOS && (n = strlen (s)) == 0))
    return 0;
  if (p0 < this->len && n <= (nmax = this->len - p0))
    {
      for (nmax -= n - 1, u = this->ptr + p0;
           (t = (char *) memchr (u, *s, nmax)) != 0;
           nmax -= t - u + 1, u = t + 1)
        if (memcmp (t, s, n) == 0)
          return t - this->ptr;
    }
  return NPOS;
}

/* DSrfind: Similar to DSfind, except that the string is searched backwards.  */
size_t
DSrfind (STRING_T * this, char *s, size_t p0, size_t n)
{
  char *t;

  if (n == 0 || (n == NPOS && (n = strlen (s)) == 0))
    return 0;
  if (n <= this->len)
    {
      for (t = this->ptr + (p0 < this->len - n ? p0 : this->len - n);; --t)
        {
          if (*t == *s && memcmp (t, s, n) == 0)
            return t - this->ptr;
          else if (t == this->ptr)
            break;
        }
    }
  return NPOS;
}

/* DSfind_first_of: Find the first character within this (starting at position
   p0 within this) that matches any of the first n characters of string s.  */
size_t
DSfind_first_of (STRING_T * this, char *s, size_t p0, size_t n)
{
  char *t, *u;

  if (n == 0 || (n == NPOS && (n = strlen (s)) == 0))
    return 0;
  if (p0 < this->len)
    {
      u = this->ptr + this->len;
      for (t = this->ptr + p0; t < u; t++)
        if (memchr (s, *t, n) != 0)
          return t - this->ptr;
    }
  return NPOS;
}

/* DSfind_last_of: Find the last character within this (starting at position
   p0 within this) that matches any of the first n characters of string s.  */
size_t
DSfind_last_of (STRING_T * this, char *s, size_t p0, size_t n)
{
  char *t;

  if (n == 0 || (n == NPOS && (n = strlen (s)) == 0))
    return 0;
  if (0 < this->len)
    {
      for (t = this->ptr + (p0 < this->len ? p0 : this->len - 1);; t--)
        {
          if (memchr (s, *t, n) != 0)
            return t - this->ptr;
          else if (t == this->ptr)
            break;
        }
    }
  return NPOS;
}

/* DSfind_first_not_of: Similar to DSfind_first_of, except that we're finding
   the first character that matches none of the first n characters of s.  */
size_t
DSfind_first_not_of (STRING_T * this, char *s, size_t p0, size_t n)
{
  char *t, *u;

  if (n == 0 || (n == NPOS && (n = strlen (s)) == 0))
    return 0;
  if (p0 < this->len)
    {
      u = this->ptr + this->len;
      for (t = this->ptr + p0; t < u; t++)
        if (memchr (s, *t, n) == 0)
          return t - this->ptr;
    }
  return NPOS;
}

/* DSfind_last_not_of: Similar to DSfind_first_not_of, except that we're
   finding the last character that matches none of the first n characters of
   string s.  */
size_t
DSfind_last_not_of (STRING_T * this, char *s, size_t p0, size_t n)
{
  char *t;

  if (n == 0 || (n == NPOS && (n = strlen (s)) == 0))
    return 0;
  if (0 < this->len)
    {
      for (t = this->ptr + (p0 < this->len ? p0 : this->len - 1);; t--)
        {
          if (memchr (s, *t, n) == 0)
            return t - this->ptr;
          else if (t == this->ptr)
            break;
        }
    }
  return NPOS;
}

/* DScomparechar: Compare the substring starting at this[p0] with a repeated
   character.  */
int
DScomparechar (STRING_T * this, int c, size_t p0, size_t ns)
{
  size_t n;
  char *s, *t;

  if (this->len < p0)
    DSxran ();
  n = this->len - p0;
  for (s = this->ptr + p0, t = s + (n < ns ? n : ns); s < t; s++)
    if (*s != c)
      return (*(unsigned char *) s < (unsigned char) c ? -1 : 1);
  return (n < ns ? -1 : n > ns);
}

/* DScomparecstr: Compare the substring starting at this[p0] with a string s
   for ns characters.  */
int
DScomparecstr (STRING_T * this, char *s, size_t p0, size_t ns)
{
  size_t n;
  int ans;

  if (this->len < p0)
    DSxran ();
  n = this->len - p0;
  if (ns == NPOS)
    ns = strlen (s);
  ans = memcmp (this->ptr + p0, s, n < ns ? n : ns);
  return ans ? ans : n < ns ? -1 : n > ns;
}

/* DScompare: Compare two STRING_T *'s.  */
int
DScompare (STRING_T * this, STRING_T * str, size_t p0, size_t ns)
{
  size_t n;
  int ans;

  if (this->len < p0)
    DSxran ();
  n = this->len - p0;
  if (DSlength (str) < ns)
    ns = DSlength (str);
  ans = memcmp (this->ptr + p0, DScstr (str), n < ns ? n : ns);
  return ans ? ans : n < ns ? -1 : n > ns;
}

/* DScstr: Convert a STRING_T* to a regular C string (char *).  */
char *
DScstr (STRING_T * this)
{
  return this->ptr ? this->ptr : "";
}

/* DSlength: Return the length of a STRING_T*. */
size_t
DSlength (STRING_T * this)
{
  return this->len;
}

/* DSresize:  Resize a string to n characters.  Use c as the fill-in 
   character if resizing to a bigger size.  */
void
DSresize (STRING_T * this, size_t n, int c)
{
  if (n < this->len)
    DSremove (this, n, NPOS);
  else
    DSappendchar (this, c, n - this->len);
}

/* DSreserve:  Return how many bytes are in reserve.  */
size_t
DSreserve (STRING_T * this)
{
  return this->res;
}

/* DSset_reserve:  Set the number of reserve bytes.  This function only works 
   if the string doesn't contain any data.  */
void
DSset_reserve (STRING_T * this, size_t n)
{
  if (this->ptr == 0)
    this->res = n;
}

/* DSsubstr: Create a string equal to the substring of this at position p 
   with length n.  */
STRING_T *
DSsubstr (STRING_T * this, size_t p, size_t n)
{
  STRING_T *s = DScreate ();

  DSassign (s, this, p, n);
  return s;
}

/* Invoke "dynarray.h" to get us array-of-string functions. */
#define T               STRING_T
#define TS              DAS
#define Tassign(x,y)    DSassign(x,y,0,NPOS)
#define Tctor(x)        DSctor(x)
#define Tdtor(x)        DSdtor(x)
#undef  PROTOS_ONLY
#include "dynarray.h"
#undef  T
#undef  TS
#undef  Tassign
#undef  Tctor
#undef  Tdtor

/* END OF FILE */
