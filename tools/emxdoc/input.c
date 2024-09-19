/* input.c -- Read and tokenize the input file
   Copyright (c) 1993-1999 Eberhard Mattes

This file is part of emxdoc.

emxdoc is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

emxdoc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with emxdoc; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "emxdoc.h"
#include "cond.h"

#define COND_STACK_SIZE         8

struct cond
{
  int start_line;
  int true;
  int else_seen;
};

struct recode_table
{
  enum enc inp;
  enum enc out;
  uchar tab[256];
};

static struct cond cond_stack[COND_STACK_SIZE];
static int cond_sp;

static const uchar *char_table[ENCODINGS] =
{
  /* ENC_CP850 */
  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
  "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\xa0"
  "\xa1\xa2\xa3\xa4\xa5\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb5\xb6\xb7"
  "\xb8\xbd\xbe\xc6\xc7\xcf\xd0\xd1\xd2\xd3\xd4\xd6\xd7\xd8\xdd\xde"
  "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xf1\xf3"
  "\xf4\xf5\xf6\xfa\xfb\xfc\xfd",

  /* ENC_ISO8859_1 */
  "\xc7\xfc\xe9\xe2\xe4\xe0\xe5\xe7\xea\xeb\xe8\xef\xee\xec\xc4\xc5"
  "\xc9\xe6\xc6\xf4\xf6\xf2\xfb\xf9\xff\xd6\xdc\xf8\xa3\xd8\xd7\xe1"
  "\xed\xf3\xfa\xf1\xd1\xbf\xae\xac\xbd\xbc\xa1\xab\xbb\xc1\xc2\xc0"
  "\xa9\xa2\xa5\xe3\xc3\xa4\xf0\xd0\xca\xcb\xc8\xcd\xce\xcf\xa6\xcc"
  "\xd3\xdf\xd4\xd2\xf5\xd5\xb5\xfe\xde\xda\xdb\xd9\xfd\xdd\xb1\xbe"
  "\xb6\xa7\xf7\xb7\xb9\xb3\xb2"
};

static const uchar chars_iso8859_1[] = "";

static const struct recode_table *build_recode_table (enum enc inp,
                                                      enum enc out)
{
  struct recode_table *p;
  const uchar *pi, *po;
  int i;

  if (inp < 0 || inp >= ENCODINGS || out < 0 || out >= ENCODINGS)
    abort ();
  p = xmalloc (sizeof (*p));
  p->inp = inp;
  p->out = out;
  memset (p->tab, 0, 256);
  for (i = 0; i < 128; ++i)
    p->tab[i] = (uchar)i;

  pi = char_table[inp];
  po = char_table[out];
  for (i = 0; pi[i] != 0 && po[i] != 0; ++i)
    if (pi[i] != ' ' && po[i] != ' ')
      {
        if (p->tab[pi[i]] != 0)
          abort ();
        p->tab[pi[i]] = po[i];
      }
  if (pi[i] != 0 || po[i] != 0)
    abort ();
  p->tab[escape] = escape;
  return p;
}

static int recode (uchar *s, enum enc inp, enum enc out)
{
  static const struct recode_table *rt;
  const uchar *t;

  if (rt == NULL)
    rt = build_recode_table (inp, out);
  else if (inp != rt->inp || out != rt->out)
    fatal ("recode(): encoding changed");
  t = rt->tab;
  while (*s != 0)
    {
      if (t[*s] == 0)
        return *s;
      *s = t[*s];
      ++s;
    }
  return 0;
}

static enum enc find_encoding (const uchar *s)
{
  char pos[ENCODINGS];
  char neg[ENCODINGS];
  uchar c;
  int i, j;

  memset (pos, 0, ENCODINGS);
  memset (neg, 0, ENCODINGS);
  while (*s != 0)
    {
      c = *s++;
      if (c >= 0x80 && c != escape)
        for (i = 0; i < ENCODINGS; ++i)
          {
            if (strchr (char_table[i], c) == NULL)
              neg[i] = 1;
            else
              pos[i] = 1;
          }
    }

  i = 0;
  while (i < ENCODINGS && !pos[i] && !neg[i])
    ++i;
  if (i >= ENCODINGS)
    return ENC_DEFAULT;         /* No character codes >= 0x80 */

  j = -1;
  for (i = 0; i < ENCODINGS; ++i)
    if (pos[i] && !neg[i])
      {
        if (j != -1)
          return ENC_AMBIGUOUS;
        j = i;
      }
  if (j == -1)
    return ENC_UNSUPPORTED;
  return (enum enc)j;
}

static void choose_encoding (const uchar *s)
{
  enum enc e = find_encoding (s);
  if (e != ENC_DEFAULT)
    {
      if (e == ENC_AMBIGUOUS)
        fatal ("Input encoding ambiguous, please use -i");
      if (e == ENC_UNSUPPORTED)
        fatal ("Input encoding unsupported, please use -i");
      input_encoding = e;
      if (output_encoding == ENC_DEFAULT)
        output_encoding = e;
    }
}

#define ISARG(C) ((C) == '{' || (C) == '[')
#define ISARGW(C) (isspace (C) || ISARG (C))
#define ISENDW(C) (isspace (C) || (C) == 0)

#define SKIP1W(P,AT) do { (P) += (AT); if (isspace (*(P))) ++(P); } while (0)
#define SKIPW(P,AT) do { (P) += (AT); while (isspace (*(P))) ++(P); } while (0)

void read_line (void)
{
  uchar *p;
  int c1;

redo:
  ++line_no;
  if (fgets (input, sizeof (input), input_file) == NULL)
    {
      if (ferror (input_file))
        {
          perror (input_fname);
          exit (1);
        }
      end_of_file = TRUE;
      if (cond_sp >= 0)
        fatal ("%s:%d: Unterminated %cif",
               input_fname, cond_stack[cond_sp].start_line, escape);
      return;
    }
  p = strchr (input, '\n');
  if (p == NULL)
    fatal ("%s:%d: Line too long", input_fname, line_no);
  *p = 0;
  if (input_encoding == ENC_DEFAULT && output_encoding != ENC_DEFAULT)
    choose_encoding (input);
  if (input_encoding != output_encoding && input_encoding != ENC_DEFAULT)
    {
      int c = recode (input, input_encoding, output_encoding);
      if (c != 0)
        fatal ("%s:%d: unsupported character 0x%.2x",
               input_fname, line_no, c);
    }
  if (input[0] == escape)
    {
      p = input + 1;
      if (strncmp (p, "c", 1) == 0 && ISENDW (p[1]))
        goto redo;
      if (strncmp (p, "if", 2) == 0 && isspace (p[2]))
        {
          SKIPW (p, 2);
          c1 = condition (p);
          if (cond_sp + 1 >= COND_STACK_SIZE)
            fatal ("%s:%d: Conditional stack overflow", input_fname, line_no);
          ++cond_sp;
          cond_stack[cond_sp].true = c1;
          cond_stack[cond_sp].start_line = line_no;
          cond_stack[cond_sp].else_seen = FALSE;
          goto redo;
        }
      else if (strcmp (p, "else") == 0)
        {
          if (cond_sp < 0)
            fatal ("%s:%d: %celse without %cif",
                   input_fname, line_no, escape, escape);
          if (cond_stack[cond_sp].else_seen)
            fatal ("%s:%d: Multiple %celse for %cif in line %d",
                   input_fname, line_no, escape, escape,
                   cond_stack[cond_sp].start_line);
          cond_stack[cond_sp].else_seen = TRUE;
          cond_stack[cond_sp].true = !cond_stack[cond_sp].true;
          goto redo;
        }
      else if (strcmp (p, "endif") == 0)
        {
          if (cond_sp < 0)
            fatal ("%s:%d: %cendif without %cif",
                   input_fname, line_no, escape, escape);
          --cond_sp;
          goto redo;
        }
      else if (p[0] == 'h' && p[1] >= '1' && p[1] <= '0' + SECTION_LEVELS)
        {
          /* Support h1 inside if */
          if (cond_sp >= 0 && !cond_stack[cond_sp].true)
            ++ref_no;
        }
    }

  if (cond_sp >= 0 && !cond_stack[cond_sp].true)
    goto redo;

  p = input;
  while (isspace (*p))
    ++p;
  if (*p == 0)
    input[0] = 0;
}


void open_input (const char *name)
{

  line_no = 0; end_of_file = FALSE; cond_sp = -1;
  input_fname = name;
  input_file = fopen (input_fname, "rt");
  if (input_file == NULL)
    {
      perror (input_fname);
      exit (1);
    }
}


static void invalid_tag (void)
{
  fatal ("%s:%d: Invalid tag", input_fname, line_no);
}


int parse_tag (const uchar **ptr)
{
  const uchar *p;

  p = *ptr;
  if (*p != escape)
    return FALSE;
  ++p;                          /* Skip escape character */
  tg_flags = 0;
  switch (*p)
    {
    case '.':
      tg_tag = TAG_FULLSTOP;
      ++p;
      break;

    case 'b':
      if (strncmp (p, "bf", 2) == 0 && ISARGW (p[2]))
        {
          tg_tag = TAG_STYLE;
          tg_style = STYLE_BOLD;
          p += 2;
        }
      else if (strncmp (p, "break", 5) == 0 && ISENDW (p[5]))
        {
          tg_tag = TAG_BREAK;
          SKIP1W (p, 5);        /* Skip one space */
        }
      else if (strcmp (p, "bugs") == 0)
        tg_tag = TAG_BUGS;
      else
        invalid_tag ();
      break;

    case 'c':
      if (strncmp (p, "compat", 6) == 0 && isspace (p[6]))
        {
          tg_tag = TAG_COMPAT;
          p += 6;
        }
      else
        invalid_tag ();
      break;

    case 'd':
      if (strcmp (p, "description") == 0)
        tg_tag = TAG_DESCRIPTION;
      else
        invalid_tag ();
      break;

    case 'e':
      if (strncmp (p, "em", 2) == 0 && ISARGW (p[2]))
        {
          tg_tag = TAG_STYLE;
          tg_style = STYLE_EMPHASIZE;
          p += 2;
        }
      else if (strcmp (p, "enddescription") == 0)
        tg_tag = TAG_ENDDESCRIPTION;
      else if (strcmp (p, "endenumerate") == 0)
        tg_tag = TAG_ENDENUMERATE;
      else if (strcmp (p, "endexample") == 0)
        tg_tag = TAG_ENDEXAMPLE;
      else if (strcmp (p, "endheaders") == 0)
        tg_tag = TAG_ENDHEADERS;
      else if (strcmp (p, "endhtml") == 0)
        tg_tag = TAG_ENDHTML;
      else if (strcmp (p, "endindent") == 0)
        tg_tag = TAG_ENDINDENT;
      else if (strcmp (p, "enditemize") == 0)
        tg_tag = TAG_ENDITEMIZE;
      else if (strcmp (p, "endlist") == 0)
        tg_tag = TAG_ENDLIST;
      else if (strcmp (p, "endprototype") == 0)
        tg_tag = TAG_ENDPROTOTYPE;
      else if (strcmp (p, "endsamplecode") == 0)
        tg_tag = TAG_ENDSAMPLECODE;
      else if (strcmp (p, "endverbatim") == 0)
        tg_tag = TAG_ENDVERBATIM;
      else if (strcmp (p, "endtypewriter") == 0)
        tg_tag = TAG_ENDTYPEWRITER;
      else if (strcmp (p, "endipf") == 0)
        tg_tag = TAG_ENDIPF;
      else if (strcmp (p, "endlatex") == 0)
        tg_tag = TAG_ENDLATEX;
      else if (strcmp (p, "endtext") == 0)
        tg_tag = TAG_ENDTEXT;
      else if (strcmp (p, "endtable") == 0)
        tg_tag = TAG_ENDTABLE;
      else if (strcmp (p, "enumerate") == 0)
        tg_tag = TAG_ENUMERATE;
      else if (strcmp (p, "errors") == 0)
        tg_tag = TAG_ERRORS;
      else if (strcmp (p, "example") == 0)
        tg_tag = TAG_EXAMPLE;
      else
        invalid_tag ();
      break;

    case 'f':
      if (strncmp (p, "format", 6) == 0 && isspace (p[6]))
        {
          tg_tag = TAG_FORMAT;
          SKIPW (p, 6);
        }
      else if (strncmp (p, "function", 8) == 0 && isspace (p[8]))
        {
          tg_tag = TAG_FUNCTION;
          SKIPW (p, 8);
        }
      else
        invalid_tag ();
      break;

    case 'h':
      if (p[1] >= '1' && p[1] <= '0' + SECTION_LEVELS)
        {
          tg_tag = TAG_HEADING;
          tg_level = p[1] - '0';
          tg_underline = (tg_level == 1 ? '=' : '-');
          p += 2;
          while (*p != 0 && !isspace (*p))
            switch (*p++)
              {
              case 'h':
                tg_flags |= HF_HIDE;
                break;
              case 'u':
                tg_flags |= HF_UNNUMBERED;
                break;
              default:
                invalid_tag ();
              }
          SKIPW (p, 0);
        }
      else if (p[1] == '-' || p[1] == '=')
        {
          tg_tag = TAG_HEADING;
          tg_level = 0;
          tg_underline = p[1];
          SKIPW (p, 2);
        }
      else if (strncmp (p, "hpt", 3) == 0 && ISARGW (p[3]))
        {
          tg_tag = TAG_HPT;
          p += 3;
        }
      else if (strcmp (p, "headers") == 0)
        tg_tag = TAG_HEADERS;
      else if (strcmp (p, "hints") == 0)
        tg_tag = TAG_HINTS;
      else if (strcmp (p, "html") == 0)
        tg_tag = TAG_HTML;
      else if (strncmp (p, "htmlfragment", 12) == 0 && isspace (p[12]))
        {
          tg_tag = TAG_HTMLFRAGMENT;
          SKIPW (p, 12);
        }
      else if (strcmp (p, "htmlminitoc") == 0)
        tg_tag = TAG_HTMLMINITOC;
      else
        invalid_tag ();
      break;

    case 'i':
      if (p[1] >= '1' && p[1] <= '2')
        {
          tg_tag = TAG_INDEX;
          tg_level = p[1] - '0';
          SKIPW (p, 2);
        }
      else if (strcmp (p, "implementation") == 0)
        tg_tag = TAG_IMPLEMENTATION;
      else if (strcmp (p, "ipf") == 0)
        tg_tag = TAG_IPF;
      else if (strcmp (p, "ipfminitoc") == 0)
        tg_tag = TAG_IPFMINITOC;
      else if (strncmp (p, "item", 4) == 0 && ISENDW (p[4]))
        {
          tg_tag = TAG_ITEM;
          SKIPW (p, 4);
        }
      else if (strncmp (p, "index", 5) == 0 && isspace (p[5]))
        {
          tg_tag = TAG_INDEX;
          tg_level = 0;
          SKIPW (p, 5);
        }
      else if (strcmp (p, "indent") == 0)
        tg_tag = TAG_INDENT;
      else if (strcmp (p, "itemize") == 0)
        tg_tag = TAG_ITEMIZE;
      else
        invalid_tag ();
      break;

    case 'k':
      if (strncmp (p, "keyword ", 8) == 0)
        {
          tg_tag = TAG_KEYWORD;
          SKIPW (p, 8);
        }
      else
        invalid_tag ();
      break;

    case 'l':
      if (strncmp (p, "label ", 6) == 0)
        {
          tg_tag = TAG_LABEL;
          SKIPW (p, 6);
        }
      else if (strncmp (p, "language", 8) == 0 && isspace (p[8]))
        {
          tg_tag = TAG_LANGUAGE;
          SKIPW (p, 8);
        }
      else if (strcmp (p, "latex") == 0)
        tg_tag = TAG_LATEX;
      else if (strcmp (p, "list") == 0)
        tg_tag = TAG_LIST;
      else
        invalid_tag ();
      break;

    case 'm':
      if (strcmp (p, "minitoc") == 0)
        tg_tag = TAG_MINITOC;
      else
        invalid_tag ();
      break;
    case 'p':
      if (strcmp (p, "prototype") == 0)
        tg_tag = TAG_PROTOTYPE;
      else if (strncmp (p, "param", 5) == 0 && isspace (p[5]))
        {
          tg_tag = TAG_PARAM;
          SKIPW (p, 5);
        }
      else if (strncmp (p, "pa", 2) == 0 && ISARGW (p[2]))
        {
          tg_tag = TAG_STYLE;
          tg_style = STYLE_PARAM;
          p += 2;
        }
      else
        invalid_tag ();
      break;

    case 'r':
      if (strncmp (p, "ref", 3) == 0 && ISARGW (p[3]))
        {
          tg_tag = TAG_REF;
          p += 3;
        }
      else if (strncmp (p, "replace", 7) == 0 && isspace (p[7]))
        {
          tg_tag = TAG_REPLACE;
          SKIPW (p, 7);
        }
      else if (strcmp (p, "restrictions") == 0)
        tg_tag = TAG_RESTRICTIONS;
      else if (strcmp (p, "returnvalue") == 0)
        tg_tag = TAG_RETURNVALUE;
      else
        invalid_tag ();
      break;

    case 's':
      if (strncmp (p, "seealso", 7) == 0 && isspace (p[7]))
        {
          tg_tag = TAG_SEEALSO;
          p += 7;
        }
      else if (strcmp (p, "samplecode") == 0)
        tg_tag = TAG_SAMPLECODE;
      else if (strncmp (p, "samplefile", 10) == 0 && isspace (p[10]))
        {
          tg_tag = TAG_SAMPLEFILE;
          SKIPW (p, 10);
        }
      else if (strncmp (p, "set", 3) == 0 && isspace (p[3]))
        {
          tg_tag = TAG_SET;
          SKIPW (p, 3);
        }
      else if (strncmp (p, "sl", 2) == 0 && ISARGW (p[2]))
        {
          tg_tag = TAG_STYLE;
          tg_style = STYLE_SLANTED;
          p += 2;
        }
      else if (strncmp (p, "special", 7) == 0 && isspace (p[7]))
        {
          tg_tag = TAG_SPECIAL;
          SKIPW (p, 7);
        }
      else if (strncmp (p, "sy", 2) == 0 && ISARGW (p[2]))
        {
          tg_tag = TAG_STYLE;
          tg_style = STYLE_SYNTAX;
          p += 2;
        }
      else if (strncmp (p, "syntax", 6) == 0 && isspace (p[6]))
        {
          tg_tag = TAG_SYNTAX;
          SKIPW (p, 6);
        }
      else
        invalid_tag ();
      break;

    case 't':
      if (strncmp (p, "tt", 2) == 0 && ISARGW (p[2]))
        {
          tg_tag = TAG_STYLE;
          tg_style = STYLE_TTY;
          p += 2;
        }
      else if (strcmp (p, "toc") == 0)
        tg_tag = TAG_TOC;
      else if (strcmp (p, "text") == 0)
        tg_tag = TAG_TEXT;
      else if (strncmp (p, "table ", 6) == 0)
        {
          tg_tag = TAG_TABLE;
          SKIPW (p, 6);
        }
      else if (strncmp (p, "title ", 6) == 0)
        {
          tg_tag = TAG_TITLE;
          SKIPW (p, 6);
        }
      else if (strcmp (p, "typewriter") == 0)
        tg_tag = TAG_TYPEWRITER;
      else
        invalid_tag ();
      break;

    case 'u':
      if (strncmp (p, "ul", 2) == 0 && ISARGW (p[2]))
        {
          tg_tag = TAG_STYLE;
          tg_style = STYLE_UNDERLINE;
          p += 2;
        }
      else
        invalid_tag ();
      break;

    case 'v':
      if (strcmp (p, "verbatim") == 0)
        tg_tag = TAG_VERBATIM;
      else
        invalid_tag ();
      break;

    default:
      invalid_tag ();
    }
  *ptr = p;
  return TRUE;
}
