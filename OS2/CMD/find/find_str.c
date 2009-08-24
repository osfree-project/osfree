/* find_str.c */

/* Back-end to the DOS "FIND" Program */

/* Copyright (C) 1994-2002 Jim Hall <jhall@freedos.org> */
/* NLS case insensitivity support by Eric Auer 2003     */

/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> /* ltoa */
#include "kitten.h"


/* Symbolic constants */

#define MAX_STR 1024 /* max line length */


/* Functions */

/* This function prints out all lines containing a substring.  There are some
 * conditions that may be passed to the function.
 *
 * RETURN: If the string was found at least once, returns 1.
 * If the string was not found at all, returns 0.
 */

/* save some stack space: */
  unsigned char temp_str[MAX_STR];
  unsigned char this_line[MAX_STR+3]; /* extra space for \r\n\0 */

int
find_str (unsigned char *sz, int thefile,
       int invert_search, int count_lines, int number_output, int ignore_case)
{
  int i, length;
  long line_number = 0, total_lines = 0;
  unsigned char *c;
  char numbuf[16]; /* for itoa */

  (void)get_line (thefile, NULL, 0); /* init buffers */

  /* Convert to upper if needed */

  if (ignore_case)
    {
      length = strlen (sz);
      for (i = 0; i < length; i++)
        sz[i] = toupper (sz[i]);
    }

  /* Scan the file until EOF */


  while ( get_line (thefile, temp_str, sizeof(temp_str)) )
    /* ( fgets (temp_str, MAX_STR, p) != NULL ) */
    /* get_line of kitten is smaller than f-stuff overhead */
    {

      /* Remove the trailing newline, if any */


      length = strlen (temp_str);
      if (temp_str[length-1] == '\n')
        {
          temp_str[length-1] = '\0';
        }

      if (temp_str[length-1] == '\r')
        {
          temp_str[length-1] = '\0';
        }

      /* Increment number of lines */

      line_number++;
      strcpy (this_line, temp_str);
      /* (remembered unchanged version for showing) */

      /* Convert to upper if needed */

      if (ignore_case)
        {
          for (i = 0; i < length; i++)
            {
              temp_str[i] = toupper (temp_str[i]);
            }
        }

      /* Locate the substring */

      /* strstr() returns a pointer to the first occurrence in the
       string of the substring */

      /* This logic is not written well!  -jh */

      if ( (((c = strstr (temp_str, sz)) != NULL) && (!invert_search))
           || ((c == NULL) && (invert_search)) )
        {

          if (!count_lines) /* only show hits if not counting them */
            {
              if (number_output) {
                /* printf ("%ld:", line_number); */
                ltoa(line_number, numbuf, 10);
                fprintf(stderr,"%s:", numbuf);
              }

              /* Print the line of text, after adding \r\n\0 */

              c = this_line + strlen(this_line);
              c[0] = '\r';
              c[1] = '\n';
              c[2] = '\0';
              fprintf(stderr, "%s", this_line);
            }

          total_lines++;
        } /* long if */
    } /* while fgets */

  if (count_lines)
    {
      /* Just show num. lines that contain the string */
      /* printf ("%ld\n", total_lines); */
      ltoa(total_lines, numbuf, 10);
      fprintf(stderr, "%s\r\n", numbuf);
    }


 /* RETURN: If the string was found at least once, returns 1.
  * If the string was not found at all, returns 0.
  */

  return (total_lines > 0 ? 1 : 0);
}
