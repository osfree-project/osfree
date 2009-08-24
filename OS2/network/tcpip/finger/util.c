/* util.c -- Simple utility functions that everyone uses. */

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
#include <stdlib.h>
#include <config.h>
#include <general.h>

/* **************************************************************** */
/*                                                                  */
/*                      General Utility                             */
/*                                                                  */
/* **************************************************************** */

/* Array processing. */

/* Return the length of the null terminated array of pointers. */
int
array_len (array)
     char **array;
{
  register int i;

  for (i = 0; array[i]; i++);
  return (i);
}

/* Free the contents of the null terminated array, and
   then the array itself. */
/*
int
free_array (array)
     char **array;
{
  register int i;

  for (i = 0; array[i]; i++)
    free (array[i]);

  free (array);
}
*/
/* Whoops, Unix doesn't have xstrnicmp.  String functions. */

#if defined (USG) || defined (hpux)
#if !defined (index)
char *
index (string, ch)
     char *string;
     int ch;
{
  return ((char *)strchr (string, ch));
}
#endif

#if !defined (rindex)
char *
rindex (string, ch)
     char *string;
     int ch;
{
  return ((char *)strrchr (string, ch));
}
#endif
#endif /* USG || hpux */


/* Compare at most COUNT characters from string1 to string2.  Case
   doesn't matter. */
int
xstrnicmp (string1, string2, count)
  char *string1, *string2;
{
  register char ch1, ch2;

  while (count)
    {
      ch1 = *string1++;
      ch2 = *string2++;

      if (to_upper(ch1) == to_upper(ch2))
        count--;
      else
        break;
    }
  return (count);
}


/* xstrcmp (), but caseless. */
int
xstricmp (string1, string2)
  char *string1, *string2;
{
  char ch1, ch2;

  while (*string1 && *string2)
    {
      ch1 = *string1++;
      ch2 = *string2++;
      if (to_upper(ch1) != to_upper(ch2))
        return (1);
    }
  return (*string1 | *string2);
}

/* Determine if s2 occurs in s1.  If so, return a pointer to the
   match in s1.  The compare is case insensitive. */
char *
strindex (s1, s2)
  register char *s1, *s2;
{
  register int i, l = strlen (s2);
  register int len = strlen (s1);

  for (i = 0; (len - i) >= l; i++)
    if (xstrnicmp (&s1[i], s2, l) == 0)
      return (s1 + i);
  return ((char *)NULL);
}


/* Match pattern string P against string S.
   The special pattern characters are:

   " "    Matches both "." and " "

   Returns 1 on match, 0 if no match. */

#ifdef __GCC__
inline
#endif

static int
matches_p (p, s)
  char *p, *s;
{
  char ch1, ch2;

  while (*p && *s)
    {
      ch1 = *p++;
      ch2 = *s++;

      if (ch1 == '.' && ch2 == ' ')
        continue;

      if (to_upper(ch1) != to_upper(ch2))
        return 0;
    }

  /* Successful if end of pattern */
  return !*p;
}


/* Determine if PATTERN matches USERNAME.  If so, return 1.  The
   compare is case insensitive. */
int
matches_username (pattern, username)
  char *pattern, *username;
{
  int i, patternlen = strlen (pattern);
  int userlen = strlen (username);

  for (i = 0;  i <= userlen - patternlen; i++)
    if (matches_p (pattern, username + i))
      return 1;

  return 0;
}


/* Return a new string which is the concatenation of PATH and FILE. */
char *
path_concat (path, file)
  char *path, *file;
{
  char *output = (char *) xmalloc (2 + strlen (path) + strlen (file));
  register int i;

  strcpy (output, path);

  for (i = strlen (output) - 1; i > -1; i--)
    if (output[i] != ' ')
      break;

  output[i + 1] = '\0';

  if (output[0] && (output[strlen (output) - 1] != '/'))
    strcat (output, "/");
  strcat (output, file);
  return (output);
}

/* **************************************************************** */
/*                                                                  */
/*                      Generate Idle Time                          */
/*                                                                  */
/* **************************************************************** */

/* Number of seconds that, below which, the terminal is not idle. */
long idle_time_threshold = 60;

/* Macro for appending possibly pluralized strings to time_buffer. */
#define ADD_TIME(str,var)\
{\
  sprintf (time_buffer + strlen (time_buffer), str, var);\
  if (var > 1) strcat (time_buffer, "s");\
  strcat (time_buffer, ", ");\
}

/* Return a static string which is the English representation of the
   amount of idle time present in ITIME.  Note that ITIME is a long. */
char *
idle_time_string (itime)
     long itime;
{
  int seconds, minutes, hours, days, weeks, months, years;
  char time_buffer[128];

  time_buffer[0] = '\0';

  if (itime < idle_time_threshold)
    return (NULL);

  hours = days = weeks = months = years = 0;

  seconds = itime % 60;
  minutes = itime / 60;

  if (minutes >= 60)    { hours = minutes / 60; minutes %= 60; }
  if (hours >= 24)      { days = hours / 24; hours %= 24; }
  if (days  >= 7)       { weeks = days / 7; days %= 7; }
  if (weeks >= 4)       { months = weeks / 4; weeks %= 4; }
  if (months >= 12)     { years = months / 12; months %= 12; }

  {
    if (years)  ADD_TIME ("%d year", years);
    if (months) ADD_TIME ("%d month", months);
    if (weeks)  ADD_TIME ("%d week", weeks);
    if (days)   ADD_TIME ("%d day", days);
  }

  sprintf (time_buffer + strlen (time_buffer), "%2d:%02d:%02d",
           hours, minutes, seconds);

  return ((char *)strcpy
          ((char *)xmalloc (1 + strlen (time_buffer)), time_buffer));
}

static void
memory_error_and_abort (what, nbytes)
  char *what;
  int nbytes;
{
  fprintf (stderr, "Fatal error: can't %s %d bytes.\n", what, nbytes);
  abort ();
}

void *
xmalloc (nbytes)
  int nbytes;
{
  char *temp = (char *)malloc (nbytes);

  if (!temp)
    memory_error_and_abort ("alloc", nbytes);

#if 0
  fprintf (stderr, "%lx: malloc\n", temp); /* DEBUG */
#endif

  return ((void *)temp);
}


void *
xrealloc (pointer, nbytes)
     void *pointer;
     int nbytes;
{
  char *temp;

  if (!pointer)
    temp = (char *)xmalloc (nbytes);
  else
    temp = (char *)realloc (pointer, nbytes);

  if (!temp)
    memory_error_and_abort (pointer ? "realloc" : "alloc", nbytes);

  return ((void *)temp);
}

#include <pwd.h>
/* Return the real person name of ENTRY. Make & stand for login name.
   Returns pointer to static buffer. */
char *
pw_real_name (entry)
  struct passwd *entry;
{
  static char *real_name_buffer = NULL;
  char *t, *s, separator_char;
  int nchars_expanded;

  t = entry->pw_gecos;

  while (*t && *t != ';' && *t != ',') t++;
  separator_char = *t;
  *t = '\0';

  /* Free old buffer */
  if (real_name_buffer)
    {
      free (real_name_buffer);
      real_name_buffer = NULL;
    }

  /* Calculate size of resultant real-name string */
  for (nchars_expanded = 0, s = entry->pw_gecos; *s; s++)
    if (*s == '&')
      nchars_expanded += strlen (entry->pw_name);
    else
      nchars_expanded++;

  /* Don't alloc one char */
  if (!nchars_expanded)
    return "";

  /* Allocate resultant string and create long name field. */
  real_name_buffer = (char *)xmalloc (nchars_expanded + 1);

  for (t = entry->pw_gecos, s = real_name_buffer; *t; *t++)
    if (*t == '&')
      {
        strcpy (s, entry->pw_name);
        *s = (*s >= 'a' && *s <= 'z' ? *s - 32 : *s);
        s += strlen (s);
      }
    else
      *s++ = *t;

  *s = 0;

  /* Restore original entry */
  *t = separator_char;

  return real_name_buffer;
}


/* Remove file name suffixes. Returns malloced string. */
static char *
strip_suffixes (s)
  char *s;
{
  char *new, *s1, *d;

  for (s1 = s; *s1 && *s1 != '.'; s1++);

  new = (char *)malloc (s1 - s + 1);

  for (d = new; s < s1; *d++ = *s++);

  *d = 0;

  return new;
}


/* Return basename sans suffixes. Returns malloced string. */
char *
baseprefix (filename)
  char *filename;
{
  char *s;

  for (s = filename + strlen (filename) - 1; s > filename; s--)
    if (*s == '/')
      return strip_suffixes (s + 1);

  return strip_suffixes (filename);
}


/* Return freshly allocated copy of S. */
char *
xstrdup (s)
  char *s;
{
  char *tmp = xmalloc (strlen (s) + 1);

  strcpy (tmp, s);

  return tmp;
}


/* Test if string is name of console */
int
is_console (s)
  char *s;
{
  int n = strlen (s);

  if (s[n-1] == '*')
    n--;

  return (!xstrnicmp (s, "console", n)
          || !strncmp (s, ":0", n)
          || !strncmp (s, "hty", 3));
}


/* Strip domain part from name. Returns fresh string. */
char *
sans_domain (fqdn_or_host)
  char *fqdn_or_host;
{
  char *tail, *hostpart;

  for (tail = fqdn_or_host; *tail && *tail != '.'; tail++);

  hostpart = xmalloc (tail - fqdn_or_host + 1);

  if (tail-fqdn_or_host)
    strncpy (hostpart, fqdn_or_host, tail - fqdn_or_host);

  hostpart[tail - fqdn_or_host] = 0;

  return hostpart;
}


#undef free
void
xfree (blk)
  void *blk;
{
#if 0
  fprintf (stderr, "%lx: free\n", blk); /* DEBUG */
#endif

  free (blk);
}
