/* get_line - this function will read a string from the file.  The
   string may be broken across several lines using a "continue_ch" as
   the final character on a line. */

/* THIS FUNCTION WILL malloc() MEMORY FOR THE STRING.  THE CALLING
   FUNCTION MUST free() THE STRING!! */

/* Copyright (C) 2000 Jim Hall <jhall1@isd.net> */

/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>			/* malloc */

char *
get_line (FILE *pfile, int continue_ch)
{
  char *str;				/* string that we will return   */
  char *tmp_str;
  int str_len;				/* current length of the string */
  int str_size;				/* malloc size of the string    */
  int ch;
  int last_ch;

  /* this function will malloc memory for the string.  the calling
     function must free() the string. */

  str_len = 0;
  str_size = 10;
  str = malloc (sizeof (char) * str_size);

  if (!str)
    {
      /* failed! */
      return (str);
    }

  /* now, read the string */

  last_ch = '\0';
  while ((ch = fgetc (pfile)) != EOF)
    {
      /* do we have enough room in the str for this ch? */

      if (str_len >= str_size)
	{
	  /* reallocate memory */

	  str_size *= 2;
	  tmp_str = realloc (str, sizeof (char) * str_size);

	  if (tmp_str)
	    {
	      /* move the pointer */
	      str = tmp_str;
	    }

	  else
	    {
	      /* failure!  return what we have */
	      return (str);
	    }
	}

      /* add the ch to the str */

      if (ch == '\n')
	{
	  /* is the string terminated? */

	  if (last_ch == continue_ch)
	    {
	      /* string is continued on next line ... ignore this ch
                 and erase last_ch in the string */
	      str_len--;
	    }

	  else
	    {
	      /* string is terminated.  return it. */
	      str[str_len++] = '\0';
	      return (str);
	    }
	}

      else
	{
	  str[str_len++] = ch;
	  last_ch = ch;
	}

    } /* while */

  /* we hit eof without eol.  return what we have. */

  return (NULL);
}
