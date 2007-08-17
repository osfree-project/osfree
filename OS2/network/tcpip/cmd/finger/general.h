/* general.h -- Generally useful macros and defines. */

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

#if !defined (_GENERAL_H_)
#define _GENERAL_H_

/* An 8-bit byte. */
#ifndef byte
#define byte unsigned char
#endif

/* So I can say (Function *)foo; */
typedef int Function ();

/* Like malloc () and realloc (), but abort if out of memory. */
void *xmalloc (), *xrealloc ();

/* Wrapper for gethostname() */
char *xgethostname (), *getservhost(), *getforwardhost();

/* Macro returns non-zero if C is a Tab, Space or Newline. */
#define whitespace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')

/* Macro returns non-zero if C is a digit character. */
#define digit(c) ((c) >= '0' && (c) <= '9')

#define lowercase_p(c) (((c) > ('a' - 1) && (c) < ('z' + 1)))
#define uppercase_p(c) (((c) > ('A' - 1) && (c) < ('Z' + 1)))
#define pure_alphabetic(c) (lowercase_p(c) || uppercase_p(c))

#ifndef to_upper
#define to_upper(c) (lowercase_p(c) ? ((c) - 32) : (c))
#define to_lower(c) (uppercase_p(c) ? ((c) + 32) : (c))
#endif

#define label_character(c) \
  (whitespace (c) || pure_alphabetic (c) || index ("-_,/", (c)))

#ifndef savestring
#define savestring(s) (char *)strcpy ((char *) xmalloc (1 + strlen (s)), (s))
#endif

#include <config.h>

#endif /* _GENERAL_H_ */
