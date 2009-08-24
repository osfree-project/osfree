/* edlin.c -- main file for edlin

  AUTHOR: Gregory Pietsch <gpietsch@comcast.net>

  DESCRIPTION:

  This file is the main file for edlin, an edlin-style line editor.

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
#include <stdio.h>
#include "dynstr.h"
#include "edlib.h"
#define EXTERN /* force a declaration */
#include "msgs.h"
#ifdef USE_CATGETS
#include "nl_types.h"
#endif
#ifdef USE_KITTEN
#include "kitten.h"
#endif


/* typedefs */
#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

/* static variables */

long current_line = 1L;
unsigned page_size = 23;
int exiting = 0;
char *current_filename = 0;

/* functions */

void
help (void)
{
  puts (G00013);
  puts (G00014);
  puts (G00015);
  puts (G00016);
  puts (G00017);
  puts (G00018);
  puts (G00019);
  puts (G00020);
  puts (G00021);
  puts (G00022);
  puts (G00023);
}

void
parse_command (char *s)
{
  char *ip = s;
  long acc = 0;
  long lp[4] = { 0UL, 0UL, 0UL, 0UL };
  char op = '+';
  int verifying = 0;
  size_t lpip = 0;

  if (*s == '\0')
    return;
  while (*ip && !isalpha (*ip) && *ip != '?')
    {
      /* parse the digits */
      if (*ip == '.')
        {
          if (acc)
            {
              /* Error: Invalid user input */
              fprintf (stderr, G00037, G00033);
              return;
            }
          acc = current_line;
          ip++;
        }
      else if (*ip == '$')
        {
          if (acc)
            {
              /* Error: Invalid user input */
              fprintf (stderr, G00037, G00033);
              return;
            }
          acc = get_last_line ();
          ip++;
        }
      else
        while (isdigit (*ip))
          acc = acc * 10 + (*ip++ - '0');
      if (lpip >= 4)
        {
          /* Error: Invalid user input */
          fprintf (stderr, G00037, G00033);
          return;
        }
      lp[lpip] += (op == '+') ? acc : -acc;
      acc = 0;
      if (*ip == '+' || *ip == '-')
        op = *ip++;
      else if (*ip == ',')
        {
          lpip++;
          ip++;
          op = '+';
        }
      else if (*ip && !(isalpha ((unsigned char) *ip)
                        || (*ip == '?' && isalpha ((unsigned char) ip[1]))))
        {
          /* Error: Invalid user input */
          fprintf (stderr, G00037, G00033);
          return;
        }
    }
  if (*ip == '?')
    {
      ip++;
      verifying = 1;
    }
  /* at this point, *ip should be pointing to '\0' or the command character */
  switch (tolower ((unsigned char) (*ip)))
    {
    case '\0':                  /* amend a single line or show help */
      if (lp[0] == 0 && verifying)
        help ();
      else if (lp[0])
        {
          current_line = lp[0];
          modify_line (current_line - 1);
        }
      else
        {
          /* Error: Invalid user input */
          fprintf (stderr, G00037, G00033);
          return;
        }
      break;
    case 'c':                   /* copy */
      if (lp[0] == 0 || lp[1] == 0)
        lp[0] = lp[1] = current_line;
      if (lp[2] == 0)
        {
          /* Error: Invalid user input */
          fprintf (stderr, G00037, G00033);
          return;
        }
      if (lp[3] == 0)
        lp[3] = 1;
      copy_block (lp[0] - 1, lp[1] - 1, lp[2] - 1, (size_t) lp[3]);
      break;
    case 'd':                   /* delete */
      if (lp[1] == 0)
        lp[1] = (lp[0]) ? lp[0] : current_line;
      if (lp[0] == 0)
        lp[0] = current_line;
      delete_block (lp[0] - 1, lp[1] - 1);
      break;
    case 'm':                   /* move */
      if (lp[0] == 0)
        lp[0] = current_line;
      if (lp[1] == 0 || lp[2] == 0)
        {
          /* invalid parameters */
          /* Error: Invalid user input */
          fprintf (stderr, G00037, G00033);
          return;
        }
      move_block (lp[0] - 1, lp[1] - 1, lp[2] - 1);
      break;
    case 'l':                   /* list */
    case 'p':                   /* print */
      if (lp[0] == 0)
        {
          lp[0] = (*ip == 'l')
            ? MAX (current_line - (int) (page_size >> 1), 1) : current_line;
        }
      if (lp[1] == 0)
        lp[1] = lp[0] + page_size - 1;
      display_block (lp[0] - 1, lp[1] - 1, current_line - 1, page_size);
      break;
    case 'q':                   /* quit */
      exiting = 1;
      break;
    case 't':                   /* transfer file */
      if (lp[0] == 0)
        lp[0] = current_line;
      ip++;
      while (*ip && isspace (*ip))
        ip++;
      transfer_file (lp[0] - 1, ip);
      current_line = lp[0];
      break;
    case 'e':                   /* write & exit */
    case 'w':                   /* write file */
      exiting = (*ip == 'e');
      ip++;
      while (*ip && isspace (*ip))
        ip++;
      if (*ip == 0 && current_filename == 0)
        /* No filename */
        fprintf (stderr, G00037, G00034);
      else
        write_file (lp[0] ? lp[0] : NPOS, *ip ? ip : current_filename);
      break;
    case 'i':                   /* insert */
      if (lp[0] == 0)
        lp[0] = current_line;
      current_line = insert_block (lp[0] - 1);
      break;
    case 'a':                   /* append */
      current_line = insert_block (get_last_line ());
      break;
    case 's':                   /* search */
      if (lp[0] == 0)
        lp[0] = MIN (current_line + 1, (long) get_last_line ());
      if (lp[1] == 0)
        lp[1] = get_last_line ();
      current_line = search_buffer (current_line, lp[0] - 1, lp[1] - 1,
                                    verifying, ip + 1);
      break;
    case 'r':                   /* replace */
      if (lp[0] == 0)
        lp[0] = MIN (current_line + 1, (long) get_last_line ());
      if (lp[1] == 0)
        lp[1] = get_last_line ();
      current_line = replace_buffer (current_line, lp[0] - 1, lp[1] - 1,
                                     verifying, ip + 1);
      break;
    default:
      /* Invalid user input */
      fprintf (stderr, G00037, G00033);
      break;
    }
}

/* Main function for edlin.  */
int
main (int argc, char **argv)
{
  char *s;

#if defined(USE_CATGETS) || defined(USE_KITTEN)
  /* get catalog */
  the_cat = catopen("edlin", 0);
#endif

  /* put out the copyright notice and disclaimer */
  fputs (PACKAGE_NAME " " PACKAGE_VERSION, stdout);
  puts (G00024);
  puts (G00025);
  puts (G00026);
  puts (G00027);
  puts (G00028);
  puts (G00029);
  create_buffer ();
  if (argc >= 2)
    transfer_file (0, current_filename = argv[1]);
  while (!exiting)
    {
      s = read_line ("*");
      parse_command (s);
    }
  destroy_buffer ();
#if defined(USE_CATGETS) || defined(USE_KITTEN)
  /* close catalog */
  catclose(the_cat);
#endif
  return 0;
}

/* END OF FILE */
