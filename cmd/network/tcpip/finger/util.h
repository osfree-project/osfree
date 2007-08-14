/* util.h -- Simple utility functions that everyone uses. */

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

#if !defined (_UTIL_H_)
#define _UTIL_H_

/* Return the length of the null terminated array of pointers. */
int array_len ();

/* Free the contents of the null terminated array, and then
   the array itself. */
void free_array ();

/* Compare at most COUNT characters from string1 to string2.  Case
   doesn't matter. */
int xstrnicmp ();

/* strcmp (), but caseless. */
int xstricmp ();

/* Determine if s2 occurs in s1.  If so, return a pointer to the
   match in s1.  The compare is case insensitive. */
char *strindex ();

/* Match pattern against username */
int matches_username ();

/* Return a new string which is the concatenation of PATH and FILE. */
char *path_concat ();

/* Return a  string which is the English representation of the
   amount of idle time present in ITIME.  Note that ITIME is a long. */
char *idle_time_string ();

/* Like malloc () and realloc (), but abort if out of memory. */
void *xmalloc (), *xrealloc ();

/* Return the real person name of ENTRY. */
char *pw_real_name ();

/* Return copy of string. */
char *xstrdup ();

/* Strip off domain part of FQDN, if any. */
char *sans_domain ();

/* Test if console */
int is_console ();

#endif /* _UTIL_H_ */
