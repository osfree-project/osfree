/* terminfo.c - read a terminfo entry from the command line */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2004  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * ######################################################################
 *
 * This file contains various functions dealing with different
 * terminal capabilities. It knows the difference between a vt52 and vt100
 * terminal (and much more) and is mainly used the terminal emulation
 * in the serial driver.
 */

#include <shared.h>
#include <term.h>
#include "terminfo.h"
#include "tparm.h"
#include "serial.h"

/* Current terminal capabilities. Default is "vt100".  */
struct terminfo term =
  {
    .name                = "vt100",
    .cursor_address      = "\033[%i%p1%d;%p2%dH",
    .clear_screen        = "\033[H\e[J",
    .enter_standout_mode = "\033[7m",
    .exit_standout_mode  = "\033[m"
  };

/* A number of escape sequences are provided in the string valued
   capabilities for easy encoding of characters there.  Both \E and \e
   map to an ESCAPE character, ^x maps to a control-x for any
   appropriate x, and the sequences \n \l \r \t \b \f \s give a
   newline, line-feed, return, tab, backspace, form-feed, and space.
   Other escapes include \^ for ^, \\ for \, \, for comma, \: for :,
   and \0 for null.  (\0 will produce \200, which does not terminate a
   string but behaves as a null character on most terminals, provid­
   ing CS7 is specified.  See stty(1).)  Finally, characters may be
   given as three octal digits after a \.  */

static char *
ti_unescape_memory (const char *in, const char *end)
{
  static char out_buffer[256];
  char c;
  char *out;

  out = out_buffer;
  do
    {
      c = *(in++);
      switch (c)
        {
        case '^':
          if (*in >= 'A' && *in <= 'Z')
            {
              *out = (*in) - 'A';
              in++;
            }
          else
            {
              *out = '^';
            }
          break;
        case '\\':
          c = *(in++);
          if (c >= '0' && c <= '9')
            {
              // octal number
              int n = 0;
              do
                {
                  n = (n << 4) | (c - '0');
                  c = *(in++);
                }
              while (c >= '0' && c <= '9');

              *out++ = (char)(n & 0xff);

              // redo last character
              in--;

              break;
            }

          switch (c)
            {
            case 'e':
            case 'E':
              *out++ = '\033';
              break;
            case 'n':
              *out++ = '\n';
              break;
            case 'r':
              *out++ = '\r';
              break;
            case 't':
              *out++ = '\t';
              break;
            case 'b':
              *out++ = '\b';
              break;
            case 'f':
              *out++ = '\f';
              break;
            case 's':
              *out++ = ' ';
              break;
            case '\\':
              *out++ = '\\';
              break;
            case '^':
              *out++ = '^';
              break;
            case ',':
              *out++ = ',';
              break;
            case ':':
              *out++ = ':';
              break;
            case '0':
              *out++ = '\200';
              break;
            }
          break;
        default:
          *out++ = c;
          break;
        }
    }
  while (in <= end);

  return out_buffer;
}

char *
ti_unescape_string (const char *in)
{
  return ti_unescape_memory (in, in + grub_strlen (in));
}

/* convert a memory region containing binary character into an external
 * ascii representation. The binary characters will be replaced by an
 * "ecsape notation". E.g. "033" will become "\e". */
static char *
ti_escape_memory (const char *in, const char *end)
{
  static char out_buffer[256];
  char c;
  char *out;

  out = out_buffer;
  do
    {
      c = *(in++);
      switch (c)
        {
        case '\33':
          *out++ = '\\'; *out++ = 'e'; break;
        case ' ':
          *out++ = '\\'; *out++ = 's'; break;
        case '\\':
          *out++ = '\\'; *out++ = '\\'; break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '%':
        case '+':
        case '-':
        case '*':
        case '/':
        case ';':
        case ':':
        case '{':
        case '}':
        case '[':
        case ']':
          *out++ = c; break;
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
          *out++ = '^'; *out++ = 'A' + c; break;
        default:
          *out++ = '\\';
          *out++ = ((c >> 8) & 7) + '0';
          *out++ = ((c >> 4) & 7) + '0';
          *out++ = ((c >> 0) & 7) + '0';
          break;
        }
    }
  while (in < end);

  *out++ = 0;

  return out_buffer;
}

/* convert a string containing binary character into an external ascii
 * representation. */
char *
ti_escape_string (const char *in)
{
  return ti_escape_memory (in, in + grub_strlen (in));
}

/* move the cursor to the given position starting with "0". */
void
ti_cursor_address (int x, int y)
{
  grub_putstr (grub_tparm (term.cursor_address, y, x));
}

/* clear the screen. */
void
ti_clear_screen (void)
{
  grub_putstr (grub_tparm (term.clear_screen));
}

/* enter reverse video */
void
ti_enter_standout_mode (void)
{
  grub_putstr (grub_tparm (term.enter_standout_mode));
}

/* exit reverse video */
void
ti_exit_standout_mode (void)
{
  grub_putstr (grub_tparm (term.exit_standout_mode));
}

/* set the current terminal emulation to use */
void
ti_set_term (const struct terminfo *new)
{
  grub_memmove (&term, new, sizeof (struct terminfo));
}

/* get the current terminal emulation */
void
ti_get_term(struct terminfo *copy)
{
  grub_memmove (copy, &term, sizeof (struct terminfo));
}
