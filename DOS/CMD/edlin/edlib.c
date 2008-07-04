/* edlib.c -- block manipulation routines for edlin

  AUTHOR: Gregory Pietsch <gpietsch@comcast.net>

  DESCRIPTION:

  This file contains block manipulation routines for edlin, an 
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
#include <ctype.h>
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>             /* need access */
#endif
#if defined(_MSC_VER) || defined(HAVE_IO_H)
#include <io.h>                 /* need access */
#ifdef _MSC_VER
#define access _access          /* silly Microsoft naming convention */
#define unlink _unlink
#endif
#ifndef HAVE_ACCESS
#define HAVE_ACCESS
#endif
#ifndef HAVE_UNLINK
#define HAVE_UNLINK
#endif
#endif
#include "dynstr.h"
#include "msgs.h"

/* typedefs */

/* macros */

#ifndef F_OK
#define F_OK 0
#endif
#if defined(HAVE_UNLINK) && defined(HAVE_LINK) && !defined(HAVE_RENAME)
#define rename(x, y) (link(x,y)?(-1):(unlink(x)))
#endif
#define FILENAME_DELIMITERS     ":/\\"
/* static variables */

DAS_ARRAY_T *buffer = 0;

/* functions */

#ifndef __STDC__
#ifndef HAVE_STRCHR
/* no strchr(), so roll our own */
#ifndef OPTIMIZED_FOR_SIZE
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
 /* Nonzero if X is not aligned on an "unsigned long" boundary.  */
#ifdef ALIGN
#define UNALIGNED(X) ((unsigned long)X&(sizeof(unsigned long)-1))
#else
#define UNALIGNED(X) 0
#endif
 /* Null character detection.  */
#if ULONG_MAX == 0xFFFFFFFFUL
#define DETECTNULL(X) (((X)-0x01010101UL)&~(X)&0x80808080UL)
#elif ULONG_MAX == 0xFFFFFFFFFFFFFFFFUL
#define DETECTNULL(X) (((X)-0x0101010101010101UL)&~(X)&0x8080808080808080UL)
#else
#error unsigned long is not 32 or 64 bits wide
#endif
#if UCHAR_MAX != 0xFF
#error char is not 8 bits wide
#endif
 /* Detect whether the character used to fill mask is in X */
#define DETECTCHAR(X,MASK) (DETECTNULL(X^MASK))
#endif /* OPTIMIZED_FOR_SIZE */
static char *
strchr (const char *s, int c)
{
  char pc = (char) c;
#ifndef OPTIMIZED_FOR_SIZE
  unsigned long *ps, mask = 0;
  size_t i;

  /* If s is unaligned, punt into the byte search loop.  This should be
     rare.  */
  if (!UNALIGNED (s))
    {
      ps = (unsigned long *) s;
      for (i = 0; i < sizeof (unsigned long); i++)
        mask =
          ((mask << CHAR_BIT) + ((unsigned char) pc & ~(~0 << CHAR_BIT)));
      /* Move ps a block at a time. */
      while (!DETECTNULL (*ps) && !DETECTCHAR (*ps, mask))
        ps++;
      /* Pick up any residual with a byte searcher.  */
      s = (char *) ps;
    }
#endif
  /* The normal byte-search loop.  */
  while (*s && *s != pc)
    s++;
  return *s == pc ? (char *) s : 0;
}

#ifndef OPTIMIZED_FOR_SIZE
#undef UNALIGNED
#undef DETECTNULL
#undef DETECTCHAR
#endif /* OPTIMIZED_FOR_SIZE */
#endif /* HAVE_STRCHR */
#endif /* __STDC__ */

#ifdef SHIFT_JIS
 /* This Borland-specific code is to counter a perceived bug in FreeDOS's 
    kbhit() routine.  This was from the Japanese port.  */
#include <conio.h>              /* for getch() */
#include <dos.h>                /* for int86() */
#ifdef HAVE_JCTYPE_H
#include <jctype.h>             /* for iskanji() */
#else
 /* No iskanji(), so roll our own.

    The "Shift-JIS" multibyte encoding for Kanji is a two-character code. The
    first character must be in the intervals [0x81..0x9F] or [0xE0..0xFC].
    Any other character is a single character. The second character must be
    in the interval [0x40..0x7E, 0x80..0xFC].

    The iskanji() function detects whether the first character is in the above
    range.  */
int
iskanji (int x)
{
  return (x >= 0x81 && x <= 0x9F) || (x >= 0xE0 && x <= 0xFC);
}
#endif

static int
kbhit_f (void)
{
  union REGS reg;

  reg.x.ax = reg.x.bx = reg.x.cx = reg.x.dx = reg.x.si = reg.x.di =
    reg.x.flags = 0;
  reg.h.ah = 0x11;
  int86 (0x16, &reg, &reg);     /* Check keyboard */
  return (reg.x.flags & 0x40) ? 0 : -1; /* 0x40 is the zero flag; it is set if 
                                           a character is waiting to be read. */
}

#endif /* SHIFT_JIS */

/* file_exists - returns whether a file exists or not.  */
static int
file_exists (char *filename)
{
#ifdef HAVE_ACCESS
  return access (filename, F_OK) == 0;
#else
  /* Do a reasonable job of faking it on non-Posix systems. */
  FILE *f = fopen (filename, "r");

  if (f)
    {
      fclose (f);
      return 1;
    }
  else
    return 0;
#endif
}

/* make_bakfile - make a backup file */
static void
make_bakfile (char *filename)
{
  STRING_T *s = 0;
  size_t pos, dotpos;
  static char dot[2] = { '.', '\0' };   /* to foil cstrings */
  static char bak[5] = { '.', 'b', 'a', 'k', '\0' };

  if (!file_exists (filename))
    return;
  s = DScreate ();
  DSassigncstr (s, filename, NPOS);
  pos = DSfind_last_of (s, FILENAME_DELIMITERS, NPOS, NPOS);
  dotpos = DSfind (s, dot, pos + 1, NPOS);
  if (dotpos != NPOS)
    DSresize (s, dotpos, 0);
  DSappendcstr (s, bak, NPOS);
  rename (filename, DScstr (s));
  DSdestroy (s);
}

/* commands */

/* transfer_file - merges the contents of a file on disk with a file in memory
 */
void
transfer_file (unsigned long line, char *filename)
{
  STRING_T *s = DScreate ();
  FILE *f;
  int c;
  unsigned long init_line = line;

  if (line > DAS_length (buffer))
    {
      puts (G00003);
      return;
    }
  if ((f = fopen (filename, "r")))
    {
      while ((c = getc (f)) != EOF)
        {
          if (c == '\n')
            {
              /* add s to buffer and reset s */
              DAS_insert (buffer, line++, s, 1, 1);
              DSresize (s, 0, '\0');
            }
          else
            DSappendchar (s, c, 1);
        }
      fclose (f);
    }
  printf ((line - init_line == 1) ? G00004 : G00005, filename,
          line - init_line);
  DSdestroy (s);
}

/* write X number of lines to a file */
void
write_file (unsigned long lines, char *filename)
{
  FILE *f;
  size_t i;

  make_bakfile (filename);
  if ((f = fopen (filename, "w")))
    {
      i = DAS_length (buffer);
      if (lines >= i)
        lines = i;
      for (i = 0; i < lines; i++)
        {
          fputs ((const char *) DScstr (DAS_get_at (buffer, i)), f);
          fputc ('\n', f);
        }
      fclose (f);
      printf ((i == 1) ? G00006 : G00007, filename, (unsigned long) i);
    }
}

/* copy a block of lines elsewhere in the buffer */
void
copy_block (unsigned long line1, unsigned long line2,
            unsigned long line3, size_t count)
{
  DAS_ARRAY_T *s = DAS_create ();
  size_t numlines = DAS_length (buffer);

  if (line1 >= numlines || line2 >= numlines || line3 > numlines ||
      (line1 < line3 && line3 <= line2))
    puts (G00003);
  else
    {
      DAS_subarray (buffer, s, line1, line2 - line1 + 1);
      DAS_insert (buffer, line3, DAS_base (s), DAS_length (s), 1);
    }
  DAS_destroy (s);
}

/* delete a block from line1 to line2 */
void
delete_block (unsigned long line1, unsigned long line2)
{
  size_t numlines = DAS_length (buffer);

  if (line1 > numlines)
    line1 = numlines - 1;
  if (line2 > numlines)
    line2 = numlines - 1;
  if (line1 > line2)
    puts (G00003);
  else
    DAS_remove (buffer, line1, line2 - line1 + 1);
}

/* move the block from line1 to line2 to immediately before line3 */
void
move_block (unsigned long line1, unsigned long line2, unsigned long line3)
{
  DAS_ARRAY_T *s = DAS_create ();
  size_t numlines = DAS_length (buffer);

  if (line1 >= numlines || line2 >= numlines || line3 > numlines
      || (line1 < line3 && line3 <= line2))
    puts (G00003);
  else
    {
      numlines = line2 - line1 + 1;
      DAS_subarray (buffer, s, line1, numlines);
      if (line3 >= line2)
        {
          DAS_insert (buffer, line3, DAS_base (s), numlines, 1);
          DAS_remove (buffer, line1, numlines);
        }
      else
        {
          DAS_remove (buffer, line1, numlines);
          DAS_insert (buffer, line3, DAS_base (s), numlines, 1);
        }
    }
  DAS_destroy (s);
}

/* read a line from stdin */
char *
read_line (char *prompt)
{
  static STRING_T *ds = 0;
  int c;
#ifdef SHIFT_JIS
  size_t ds_length = 0;
#endif

  if (ds == 0)
    ds = DScreate ();
  DSresize (ds, 0, 0);
  fputs (prompt, stdout);
  fflush (stdout);
#ifndef SHIFT_JIS
  /* Normal terminal input. Assumes that I don't have to handle control 
     characters here.  */
  while ((c = getchar ()) != EOF && c != '\n')
    DSappendchar (ds, c, 1);
#else /* SHIFT_JIS */
  /* Rolling our own getchar loop here. The thing to watch out for is that a
     backspace has to destroy BOTH characters of a Shift-JIS code and that a
     carriage return is equivalent to a newline.  */
  do
    {
      while (!kbhit_f ())
        ;                       /* key wait */
      c = getch ();
      if (c == '\r')
        c = '\n';
      if (c == '\b')
        {
          /* handle backspace */
          ds_length = DSlength (ds);
          if (ds_length >= 2 && iskanji (DSgetat (ds, ds_length - 2)))
            {
              DSresize (ds, ds_length - 2, 0);
              fputs ("\b \b\b \b", stdout);
            }
          else if (ds_length > 0)
            {
              DSresize (ds, ds_length - 1, 0);
              fputs ("\b \b", stdout);
            }
        }
      else
        {
          /* normal character */
          putchar (c);
          DSappendchar (ds, c, 1);
        }
      fflush (stdout);
    }
  while (c != EOF && c != '\n');
#endif /* SHIFT_JIS */
  return DScstr (ds);
}

/* display a block of text */
void
display_block (unsigned long first_line,
               unsigned long last_line,
               unsigned long current_line, size_t page_size)
{
  unsigned long i;
  size_t lines_written;
  for (i = first_line, lines_written = 0;
       i <= last_line && i < DAS_length (buffer); i++)
    {
      printf (G00008, i + 1, i == current_line ? '*' : ' ',
              DScstr (DAS_get_at (buffer, i)));
      lines_written++;
      if (lines_written == page_size && i != last_line)
        {
          read_line (G00009);
          lines_written = 0;
        }
    }
}

/* translate_string - translate a string with escapes into regular string */
static STRING_T *
translate_string (char *s, int tc)
{
  static STRING_T *r = 0;
  /* The following are arrays of characters to foil cstrings, as 
     they are not to be translated.  */
  static char escs[] = {
    'a', 'b', 'e', 'f', 't', 'v', '\\', '\'', '\"', '.'
  };
  static char xlat[] = {
    '\a', '\b', '\033', '\f', '\t', '\v', '\\', '\'', '\"', '.'
  };
  static char xdigs[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
  };
  char *p;
  int x;
#ifdef SHIFT_JIS
  char *org = s;                /* The original place in the string. */
  int iskj;                     /* The iskanji() return value. */
#endif /* SHIFT_JIS */

  if (r == 0)
    r = DScreate ();
  DSresize (r, 0, 0);
  while (*s && *s != tc)
    {
#ifndef SHIFT_JIS
      if (*s == '\\')
#else /* SHIFT_JIS */
      iskj = (org < s) && iskanji (s[-1]);
      if ((*s == '\\') && !(iskj))
#endif /* SHIFT_JIS */
        {
          /* found an escape */
          s++;
          if ((p = strchr (escs, *s)) != 0)
            DSappendchar (r, xlat[p - escs], 1);
          else if (*s == 'x')
            {
              /* handle hex digits */
              s++;
              x = 0;
              while (isxdigit ((unsigned char) *s))
                {
                  p = strchr (xdigs, tolower ((unsigned char) *s));
                  x <<= 4;
                  x |= (p - xdigs);
                  s++;
                }
              s--;
              DSappendchar (r, (x & 255), 1);
            }
          else if (*s == 'd')
            {
              /* handle decimal digits */
              s++;
              x = 0;
              while (isdigit ((unsigned char) *s))
                x = (x * 10) + (*s++ - '0');
              s--;
              DSappendchar (r, (x & 255), 1);
            }
          else if (*s >= '0' && *s <= '7')
            {
              /* handle octal digits */
              x = 0;
              while (isdigit ((unsigned char) *s))
                {
                  x <<= 3;
                  x |= (*s - '0');
                  s++;
                }
              s--;
              DSappendchar (r, (x & 255), 1);
            }
          else if (*s == '^')
            {
              /* control character */
              s++;
              x = toupper ((unsigned char) *s) ^ 64;
              DSappendchar (r, x, 1);
            }
        }
      else
        DSappendchar (r, *s, 1);
      s++;
    }
  return r;
}

/* modify_line - modify a line in the buffer */
void
modify_line (unsigned long line)
{
  char *new_line;
  STRING_T *xline;
  if (line > DAS_length (buffer))
    {
      puts (G00003);
      return;
    }
  display_block (line, line, line, 1);
  printf (G00010, line + 1);
  new_line = read_line ("");
  xline = translate_string (new_line, 0);
  DAS_put_at (buffer, (size_t) (line), xline);
}

/* insert_block - go into insert mode */
unsigned long
insert_block (unsigned long line)
{
  char *new_line;
  STRING_T *xline;
  if (line > DAS_length (buffer))
    {
      puts (G00003);
      return 0;
    }
  while (strcmp ((new_line = read_line (G00001)), ".") != 0) 
    {
      xline = translate_string (new_line, 0);
      if (DSlength(xline) > 0 && DSget_at(xline, 0) == '\032')
        break;
      DAS_insert (buffer, line++, xline, 1, 1);
    }
  return line + 1 < DAS_length (buffer) ? line + 1 : DAS_length (buffer);
}

/* search_buffer - search a buffer for a string */
unsigned long
search_buffer (unsigned long current_line,
               unsigned long line1, unsigned long line2, int verify, char *s)
{
  unsigned long line;
  STRING_T *ds;
  int q = 0;
  char *yn;
  size_t numlines = DAS_length (buffer);

  if (line1 > numlines || line2 > numlines)
    {
      puts (G00003);
      return current_line;
    }
  while (isspace ((unsigned char) *s))
    s++;
  if (*s == '\'' || *s == '\"')
    q = *s++;
  ds = translate_string (s, q);
  if (DSlength (ds) != 0)
    for (line = line1; line <= line2; line++)
      {
        if (DSfind (DAS_get_at (buffer, (size_t) line), DScstr (ds), 0,
                    DSlength (ds)) != NPOS)
          {
            display_block (line, line, line, 1);
            if (verify)
              {
                yn = read_line (G00002);
                if (*yn == 0 || strchr (YES, *yn) != 0)
                  return line + 1;
              }
            else
              return line + 1;
          }
      }
  puts (G00011);
  return current_line;
}

/* replace_buffer - search the buffer for a string, then replace it with
   another string. */
unsigned long
replace_buffer (unsigned long current_line,
                unsigned long line1, unsigned long line2, int verify, char *s)
{
  unsigned long line;
  STRING_T *ds, *ds1, *dc;
  int q = 0;
  char *yn;
  size_t origpos;

  while (isspace ((unsigned char) *s))
    s++;
  if (*s == '\'' || *s == '\"')
    q = *s++;
  else
    q = ',';
  ds = DScreate ();
  DSassign (ds, translate_string (s, q), 0, NPOS);
  /* pick off second string */
  while (*s != q && *s)
    s += (*s == '\\' ? 2 : 1);
  while (*s != ',')
    s++;
  s++;
  while (isspace ((unsigned char) *s))
    s++;
  if (*s == '\'' || *s == '\"')
    q = *s++;
  else
    q = 0;
  ds1 = DScreate ();
  DSassign (ds1, translate_string (s, q), 0, NPOS);
  if (DSlength (ds) != 0 && DScompare (ds, ds1, 0, NPOS) != 0)
    for (line = line1; line <= line2; line++)
      {
        origpos = 0;
        while ((origpos = DSfind (DAS_get_at (buffer, (size_t) line),
                                  DScstr (ds), origpos, DSlength (ds)))
               != NPOS)
          {
            dc = DScreate ();
            DSassign (dc, DAS_get_at (buffer, line), 0, NPOS);
            DSreplace (dc, origpos, DSlength (ds), ds1, 0, NPOS);
            printf (G00012, line + 1, DScstr (dc));
            if (verify)
              yn = read_line (G00002);
            if (!verify || (*yn == 0 || strchr (YES, *yn) != 0))
              {
                current_line = line + 1;
                origpos += DSlength (ds1);
                DAS_put_at (buffer, line, dc);
              }
            else
              origpos++;
            DSdestroy (dc);
          }
      }
  DSdestroy (ds);
  DSdestroy (ds1);
  return current_line;
}

/* get the last line in the buffer */
unsigned long
get_last_line (void)
{
  return DAS_length (buffer);
}

/* initialize the buffer */
void
create_buffer (void)
{
  buffer = DAS_create ();
}

/* destroy the buffer */
void
destroy_buffer (void)
{
  DAS_destroy (buffer);
  buffer = 0;
}

/* END OF FILE */
