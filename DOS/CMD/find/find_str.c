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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA
   */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> /* ltoa */
#include "kitten.h"

#ifdef __MSDOS__
#include <dos.h> /* intdosx */

/*
NLS is int 21.65 -> get tables... country=-1 is current... The data:
02 - upcase table: 128 bytes of "what is that 0x80..0xff char in uppercase?"
...
The returned buffer contents are NOT the tables but a byte followed by
a far POINTER to the actual tables!
All tables are prefixed by a word indicating the table size (128, 256...)!
Minimum DOS version for NLS stuff is 3.3 ... For table request, give BX
and DX as codepage / country, or -1 to use current. You must provide a
buffer at ES:DI and tell its length in CX. On return, CY / buffer / CX set.
Only if NLSFUNC loaded, other codepages / countries than current are avail.

From DOS 4.0 on, you also have:
Upcase char:   21.6520.dl -> dl
Upcase array:  21.6521.ds:dx.cx (string at ds:dx, length cx)
Upcase string: 21.6522.ds:dx (string terminated by 0 byte)
*/

unsigned char far * upcasetab = NULL;
#endif


/* Symbolic constants */

#define MAX_STR 1024 /* 128 */ /* max line length */


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

#ifdef __MSDOS__
    /* MAKE SURE THAT YOU USE BYTE ALIGNMENT HERE! */
    struct NLSBUF {
        char id;
        unsigned char far * content;
    } upcasebuf;
    union REGS     dosr;
    struct SREGS   doss;
#endif

  (void)get_line (thefile, NULL, 0); /* init buffers */

  /* Convert to upper if needed */

  if (ignore_case)
    {
#ifdef __MSDOS__
	if (upcasetab == NULL) {
	    /* LOAD TABLE IF NOT DONE YET BUT POSSIBLE */
            if ( (_osmajor > 3) ||
                 ((_osmajor >= 3) && (_osminor >= 3)) ) {
                dosr.x.ax = 0x6502; /* get upper case table */
                dosr.x.bx = 0xffff; /* default codepage  */
                dosr.x.dx = 0xffff; /* default country   */
                dosr.x.cx = 5;      /* buffer size   */
                doss.es = FP_SEG(&upcasebuf);
                dosr.x.di = FP_OFF(&upcasebuf);
                intdosx(&dosr,&dosr,&doss);
                if ((dosr.x.flags & 1) == 1) {
                    /* upcasetab still NULL */ /* bad luck! */
                } else {
                    /* ... CX is returned as table length ... */
                    upcasetab = upcasebuf.content; /* table pointer */
                    upcasetab++; /* skip leading word, which is */
                    upcasetab++; /* not part of the table */
                } /* end success loading uppercase table */
            } /* end if at least DOS 3.3 */
	} /* end load table */
#endif
      length = strlen (sz);
      for (i = 0; i < length; i++)
#ifndef __MSDOS__
	sz[i] = toupper (sz[i]);
#else
        if ((sz[i] < 128) || (upcasetab == NULL)) {
	    sz[i] = toupper (sz[i]);
	} else {
	    /* USE TABLE (OR, DOS 4+, DOS FUNC...) */
	    sz[i] = upcasetab[sz[i]-128];
	}
#endif
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
#ifndef __MSDOS__
	      temp_str[i] = toupper (temp_str[i]);
#else
              if ((temp_str[i] < 128) || (upcasetab==NULL)) {
	          temp_str[i] = toupper (temp_str[i]);
	      } else {
	          /* USE TABLE (OR, DOS 4+, DOS FUNC...) */
	          temp_str[i] = upcasetab[temp_str[i]-128];
	      }
#endif
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
		write(1,numbuf,strlen(numbuf));
		write(1,":",1);
	      }

	      /* Print the line of text, after adding \r\n\0 */

              c = this_line + strlen(this_line);
              c[0] = '\r';
              c[1] = '\n';
              c[2] = '\0';
	      write(1 /* stdout */, this_line, strlen(this_line));
	      /* was: puts (this_line); */
	    }

	  total_lines++;
	} /* long if */
    } /* while fgets */

  if (count_lines)
    {
      /* Just show num. lines that contain the string */
      /* printf ("%ld\n", total_lines); */
      ltoa(total_lines, numbuf, 10);
      write(1,numbuf,strlen(numbuf));
      write(1,"\r\n",2);
    }


 /* RETURN: If the string was found at least once, returns 1.
  * If the string was not found at all, returns 0.
  */

  return (total_lines > 0 ? 1 : 0);
}
