/* emxdoc.c -- Main module of emxdoc
   Copyright (c) 1993-2001 Eberhard Mattes

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


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#define EXTERN
#define INIT(X) = X
#include "emxdoc.h"
#include "html.h"
#include "ipf.h"
#include "latex.h"
#include "text.h"
#include "xref.h"
#include "cond.h"

#define VERSION "0.9d"

struct word_table
{
  unsigned hash_size;           /* hash size */
  struct word **hash_table;     /* hash table */
  int count;                    /* number of elements */
};

struct local
{
  struct word *wp;
  enum style style;
};

/* This is the return code of the program.  It is initially 0.
   nonfatal() sets it to 1. */
static int history = 0;

static int opt_r = FALSE;       /* Make output file read-only */

static const char *xref_fname = NULL;

static const char *output_fname = NULL;
static int output_flag = FALSE;

static const char *hyphenation_fname = NULL;

static uchar *line;
static size_t line_size = 0;
static size_t line_len = 0;

/* Temporary output buffer. */
static uchar output[512];

/* Syntax style. */
static enum syntax syntax_style;

static size_t elements_size = 0;
static size_t elements_count = 0;

static int max_width;

static struct local *local_stack = NULL;
static int local_size = 0;
static int local_sp;

static int section_numbers[SECTION_LEVELS];

static struct toc *toc_head = NULL;
static struct toc *toc_ptr = NULL;
static struct toc **toc_add = &toc_head;
static int toc_indent = 0;

static uchar compat[128];

static int copy_flag;

static void usage (void) NORETURN2;


/* Display an error message and stop. */

void fatal (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  vfprintf (stderr, fmt, arg_ptr);
  putc ('\n', stderr);
  exit (1);
}


/* Display an error message and set history to 1.  Don't stop. */

void nonfatal (const char *fmt, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  vfprintf (stderr, fmt, arg_ptr);
  putc ('\n', stderr);
  history = 1;
}


/* Display a warning message if opt_w is equal to or greater than
   LEVEL.  Don't change history and don't stop. */

void warning (int level, const char *fmt, ...)
{
  va_list arg_ptr;

  if (opt_w >= level)
    {
      va_start (arg_ptr, fmt);
      vfprintf (stderr, fmt, arg_ptr);
      putc ('\n', stderr);
    }
}


/* Allocate N bytes of memory.  Quit on failure.  This function is
   used like malloc(), but we don't have to check the return value. */

void *xmalloc (size_t n)
{
  void *p;

  p = malloc (n);
  if (p == NULL)
    fatal ("Out of memory");
  return p;
}


/* Change the allocation of PTR to N bytes.  Quit on failure.  This
   function is used like realloc(), but we don't have to check the
   return value. */

static void *xrealloc (void *ptr, size_t n)
{
  void *p;

  p = realloc (ptr, n);
  if (p == NULL)
    fatal ("Out of memory");
  return p;
}


/* Create a duplicate of the string S on the heap.  Quit on failure.
   This function is used like strdup(), but we don't have to check the
   return value. */

uchar *xstrdup (const uchar *s)
{
  char *p;

  p = xmalloc (strlen (s) + 1);
  strcpy (p, s);
  return p;
}


static void usage (void)
{
  fputs ("emxdoc " VERSION " -- "
         "Copyright (c) 1993-1999 by Eberhard Mattes\n\n", stderr);
  fputs ("Usage:\n", stderr);
  fputs ("  emxdoc -H [-o <output>] [-x <xref>] <input>\n", stderr);
  fputs ("  emxdoc -T [-fr] [-o <output>] <input>\n", stderr);
  fputs ("  emxdoc -L [-fr] [-o <output>] <input>\n", stderr);
  fputs ("  emxdoc -I [-acfgr] [-n <start>] [-o <output>] [-x <xref>] <input>\n", stderr);
  fputs ("  emxdoc -K [-o <output>] <input>\n", stderr);
  fputs ("  emxdoc -M [-o <output>] <input>...\n", stderr);
  fputs ("\nModes:\n\n", stderr);
  fputs ("  -H         Generate HTML file\n", stderr);
  fputs ("  -I         Generate IPF file\n", stderr);
  fputs ("  -K         Generate index file\n", stderr);
  fputs ("  -L         Generate LaTeX file\n", stderr);
  fputs ("  -M         Merge directory files\n", stderr);
  fputs ("  -T         Generate text file\n", stderr);
  fputs ("\nOptions:\n\n", stderr);
  fputs ("  -a         Concatenate instead of call .INF files\n", stderr);
  fputs ("  -b<number> Select line breaking algorithm for -t\n", stderr);
  fputs ("  -c         Use color instead of slanted font\n", stderr);
  fputs ("  -e<char>   Set escape character\n", stderr);
  fputs ("  -f         Enable French spacing\n", stderr);
  fputs ("  -g         Gather global directory data\n", stderr);
  fputs ("  -h<file>   Use hyphenation table\n", stderr);
  fputs ("  -i<enc>    Select input encoding\n", stderr);
  fputs ("  -n<start>  Set first ID number\n", stderr);
  fputs ("  -o<output> Set output file name\n", stderr);
  fputs ("  -r         Make output file read-only\n", stderr);
  fputs ("  -w<level>  Set warning level\n", stderr);
  fputs ("  -x<xref>   Use cross reference file\n", stderr);
  exit (1);
}


static void upcase (uchar *s)
{
  while (*s != 0)
    {
      *s = (uchar)toupper (*s);
      ++s;
    }
}


static void downcase (uchar *s)
{
  while (*s != 0)
    {
      *s = (uchar)tolower (*s);
      ++s;
    }
}


void write_nl (void)
{
  if (output_flag)
    {
      putc ('\n', output_file);
      if (ferror (output_file))
        {
          perror (output_fname);
          exit (1);
        }
      if (output_x > max_width)
        nonfatal ("%s:%d: Output line %d too long",
                  input_fname, line_no, output_line_no);
      output_x = 0;
      ++output_line_no;
    }
}


void write_break (void)
{
  if (output_x != 0)
    write_nl ();
}


/* P must not contain \n! */

void write_nstring (const uchar *p, size_t n)
{
  if (output_flag)
    {
      fwrite (p, n, 1, output_file);
      output_x += n;
    }
}


/* P must not contain \n! */

void write_string (const uchar *p)
{
  write_nstring (p, strlen (p));
}


/* P must not contain \n! */

void write_line (const uchar *p)
{
  write_string (p);
  write_nl ();
}

/* Output must not contain \n! */

void write_fmt (const char *fmt, ...)
{
  va_list arg_ptr;

  if (output_flag)
    {
      va_start (arg_ptr, fmt);
      output_x += vfprintf (output_file, fmt, arg_ptr);
    }
}


void write_space (void)
{
  if (output_x >= 60)
    write_nl ();
  else
    write_string (" ");
}

struct word_table *wt_new (unsigned hash_size)
{
  unsigned i;
  struct word_table *wt = xmalloc (sizeof (*wt));

  wt->hash_size = hash_size;
  wt->count = 0;
  wt->hash_table = xmalloc (hash_size * sizeof (wt->hash_table[0]));
  for (i = 0; i < hash_size; ++i)
    wt->hash_table[i] = NULL;
  return wt;
}

unsigned wt_hash (struct word_table *wt, const uchar *str)
{
  unsigned h;

  h = 0;
  while (*str != 0)
    {
      h = (h << 2) ^ *str;
      ++str;
    }
  return h % wt->hash_size;
}


struct word *wt_find (struct word_table *wt, const uchar *str, unsigned hash)
{
  struct word *wp;

  for (wp = wt->hash_table[hash]; wp != NULL; wp = wp->next)
    if (strcmp (wp->str, str) == 0)
      return wp;
  return NULL;
}


struct word *wt_add (struct word_table *wt, const uchar *str)
{
  struct word *wp;
  unsigned hash;

  hash = wt_hash (wt, str);
  wp = wt_find (wt, str, hash);
  if (wp == NULL)
    {
      wp = xmalloc (sizeof (*wp));
      wp->style = STYLE_NORMAL;
      wp->str = xstrdup (str);
      wp->ref = 0; wp->idx = 0;
      wp->database = NULL;
      wp->special = NULL;
      wp->subidx = NULL;
      wp->repl = NULL;
      wp->flags = 0;
      wp->next = wt->hash_table[hash];
      wt->hash_table[hash] = wp;
      wt->count += 1;
    }
  return wp;
}

int wt_walk (struct word_table *wt, int (*callback)(struct word *))
{
  int x = 0;
  unsigned i;
  struct word *wp;

  for (i = 0; i < wt->hash_size; ++i)
    for (wp = wt->hash_table[i]; wp != NULL; wp = wp->next)
      {
        x = callback (wp);
        if (x != 0)
          return x;
      }
  return x;
}

int wt_count (const struct word_table *wt)
{
  return wt->count;
}

unsigned word_hash (const uchar *str)
{
  return wt_hash (word_top, str);
}

struct word *word_find (const uchar *str, unsigned hash)
{
  return wt_find (word_top, str, hash);
}

struct word *word_add (const uchar *str)
{
  return wt_add (word_top, str);
}

int word_walk (int (*callback)(struct word *))
{
  return wt_walk (word_top, callback);
}

/* We do not really nest local blocks */

static void local_begin (void)
{
}


static void local_end (void)
{
  while (local_sp > 0)
    {
      --local_sp;
      local_stack[local_sp].wp->style = local_stack[local_sp].style;
    }
}


static void local_add (struct word *wp)
{
  if (local_sp >= local_size)
    {
      local_size += 256;
      local_stack = xrealloc (local_stack, local_size * sizeof (*local_stack));
    }
  local_stack[local_sp].wp = wp;
  local_stack[local_sp].style = wp->style;
  ++local_sp;
}


void format_output (const uchar *p, int may_break)
{
  switch (mode)
    {
    case 'H':
      html_output (p, may_break);
      break;
    case 'I':
      ipf_output (p, may_break);
      break;
    case 'L':
      latex_output (p, may_break);
      break;
    case 'T':
      text_output (p, may_break);
      break;
    }
}


void start_hilite (int hilite)
{
  if (hl_sp >= HL_STACK_SIZE - 1)
    fatal ("%s:%d: Highlighting stack overflow", input_fname, line_no);
  ++hl_sp;
  hl_stack[hl_sp] = hl_stack[hl_sp-1] | hilite;
  if (out)
    switch (mode)
      {
      case 'H':
        html_start_hilite ();
        break;
      case 'L':
        latex_start_hilite ();
        break;
      }
}


void end_hilite (void)
{
  if (hl_sp == 0)
    fatal ("%s:%d: Highlighting stack underflow", input_fname, line_no);
  if (out)
    switch (mode)
      {
      case 'H':
        html_end_hilite ();
        break;
      case 'L':
        latex_end_hilite ();
        break;
      }
  --hl_sp;
}


static void line_add (const uchar *src, size_t len)
{
  if (line_len + len + 1 > line_size)
    {
      line_size += 4096;
      line = xrealloc (line, line_size);
    }
  memcpy (line + line_len, src, len);
  line_len += len;
  line[line_len] = 0;
}


static struct element *add_element (void)
{
  if (elements_count >= elements_size)
    {
      elements_size += 512;
      elements = xrealloc (elements, elements_size * sizeof (*elements));
    }
  return &elements[elements_count++];
}


static struct element *add_word (const uchar *word, enum el el)
{
  struct element *ep;

  ep = add_element ();
  ep->el = el;
  ep->wp = word_add (word);
  ep->n = 0;                    /* ref */
  return ep;
}


static void make_elements_internal (const uchar *p)
{
  uchar word[512], *d, start, end, *h1;
  struct element *ep;
  struct word *wp;
  struct toc *tp;
  int n, level, paren;
  enum {other, other_dot, abbrev, abbrev_dot} state;

  state = other;
  while (*p != 0)
    {
      if (*p == escape)
        {
          state = other;
          parse_tag (&p);
          switch (tg_tag)
            {
            case TAG_STYLE:
            case TAG_REF:
            case TAG_HPT:
              start = *p;
              if (start == '{')
                end = '}';
              else if (start == '[')
                end = ']';
              else
                fatal ("%s:%d: Missing {", input_fname, line_no);
              ++p; n = 0; level = 0;
              while (*p != 0 && !(level == 0 && *p == end))
                {
                  if (n >= sizeof (word) - 1)
                    fatal ("%s%d: Argument too long", input_fname, line_no);
                  word[n++] = *p;
                  if (*p == start)
                    ++level;
                  else if (*p == end)
                    --level;
                  ++p;
                }
              if (*p != end)
                fatal ("%s:%d: Missing %c", input_fname, line_no, end);
              ++p;
              word[n] = 0;
              break;
            case TAG_BREAK:
              ep = add_element ();
              ep->el = EL_BREAK;
              ep->n = 0;
              break;
            case TAG_FULLSTOP:
              add_word (".", EL_PUNCT);
              state = other_dot;
              break;
            default:
              fatal ("%s:%d: Unexpected tag", input_fname, line_no);
            }
          switch (tg_tag)
            {
            case TAG_STYLE:
              ep = add_element ();
              ep->el = EL_STYLE;
              ep->n = tg_style;
              add_word (word, EL_WORD);
              ep = add_element ();
              ep->el = EL_ENDSTYLE;
              break;
            case TAG_REF:
              wp = word_find (word, word_hash (word));
              if (wp == NULL)
                fatal ("%s:%d: Undefined label: %s",
                       input_fname, line_no, word);
              for (tp = toc_head; tp != NULL; tp = tp->next)
                if (tp->ref == wp->ref)
                  break;
              if (tp == NULL)
                fatal ("%s:%d: Undefined label: %s",
                       input_fname, line_no, word);
              ep = add_word (tp->number, EL_WORD);
              if (ep->wp->ref == 0)
                ep->wp->ref = wp->ref;
              else if (ep->wp->ref != wp->ref)
                fatal ("%s:%d: Label redefined: %s",
                       input_fname, line_no, tp->number);
              ep->n = 1;
              break;
            case TAG_HPT:
              wp = use_reference (word);
              switch (mode)
                {
                case 'H':
                case 'I':
                  if (wp != NULL)
                    {
                      ep = add_word (word, EL_WORD);
                      ep->n = 1;
                    }
                  break;
                case 'T':
                case 'L':
                  make_elements_internal (word); /* Recursive! */
                  break;
                }
              break;
            case TAG_BREAK:
            case TAG_FULLSTOP:
              break;
            default:
              abort ();
            }
        }

      n = 0;
      while (isspace (*p))
        ++n, ++p;
      if (n != 0)
        {
          if (state == other_dot && n == 1)
            n = (opt_f ? 1 : 2);
          else if (state == abbrev_dot && n == 2)
            n = 1;
          state = other;
          ep = add_element ();
          ep->el = EL_SPACE;
          ep->n = n;
          continue;
        }

      d = word;
      while (*p == '(' || *p == '*' || *p == '`')
        *d++ = *p++;
      if (d != word)
        {
          *d = 0;
          add_word (word, EL_PUNCT);
          state = other;
          continue;
        }

      d = word; paren = 0;
      while (*p != 0 && *p != escape
             && !(isspace (*p) || *p == ',' || *p == ';'
                  || *p == '!' || *p == '?' || *p == '\''
                  || (*p == ')' && paren == 0)
                  || (*p == '[' && prototype_flag)
                  || (*p == '&' && prototype_flag)
                  || ((*p == '.' || *p == ':')
                      && (p[1] == 0 || isspace (p[1])))))
        {
          if (*p == '(')
            ++paren;
          else if (*p == ')')
            --paren;
          *d++ = *p++;
        }
      if (d != word)
        {
          *d = 0;
          wp = word_find (word, word_hash (word));
          state = (wp != NULL && (wp->flags & WF_ABBREV)) ? abbrev : other;
          if (language == LANG_GERMAN
              && wp == NULL
              && (h1 = strchr (word, '-')) != NULL)
            {
              *h1 = 0;
              if (word_find (word, word_hash (word)) != NULL)
                {
                  add_word (word, EL_WORD);
                  add_word ("-", EL_PUNCT);
                  add_word (h1 + 1, EL_WORD);
                }
              else
                {
                  *h1 = '-';
                  add_word (word, EL_WORD);
                }
            }
          else if (wp != NULL && wp->repl != NULL)
            make_elements_internal (wp->repl);
          else
            add_word (word, EL_WORD);
        }

      d = word;
      while (*p == '.' || *p == ',' || *p == ';' || *p == '\''
             || *p == ':' || *p == '!' || *p == '?' || *p == ')'
             || (*p == '[' && prototype_flag)
             || (*p == '&' && prototype_flag))
        *d++ = *p++;
      if (d != word)
        {
          *d = 0;
          add_word (word, EL_PUNCT);
          if (d[-1] == '.')
            state = ((state == abbrev && d == word + 1)
                     ? abbrev_dot : other_dot);
          else
            state = other;
        }
    }
}


static void make_elements_start (void)
{
  elements_count = 0;
}


static void make_elements_end (void)
{
  struct element *ep;

  ep = add_element ();
  ep->el = EL_END;
}


void make_elements (const uchar *p)
{
  make_elements_start ();
  make_elements_internal (p);
  make_elements_end ();
}


#define ISSYNTAXARG(C) ((C) == '*' || (C) == '#' \
                        || (syntax_style == SYNTAX_DVIDRV && (C) == '+'))

void format_string (const uchar *p, int sty, int may_break)
{
  uchar syntax[512], *d;
  const uchar *s;

  switch (sty)
    {
    case STYLE_NORMAL:
      format_output (p, may_break);
      break;
    case STYLE_BOLD:
      start_hilite (HL_BF);
      format_output (p, may_break);
      end_hilite ();
      break;
    case STYLE_SLANTED:
      start_hilite (HL_SL);
      format_output (p, may_break);
      end_hilite ();
      break;
    case STYLE_UNDERLINE:
      start_hilite (HL_UL);
      format_output (p, may_break);
      end_hilite ();
      break;
    case STYLE_TTY:
      start_hilite (HL_TT);
      format_output (p, may_break);
      end_hilite ();
      break;
    case STYLE_EMPHASIZE:
      strcpy (syntax, p);
      if (mode == 'T')
        upcase (syntax);
      start_hilite (HL_EM);
      format_output (syntax, may_break);
      end_hilite ();
      break;
    case STYLE_PARAM:
      strcpy (syntax, p);
      if (mode != 'T' || prototype_flag)
        downcase (syntax);
      start_hilite (HL_SL);
      format_output (syntax, may_break);
      end_hilite ();
      break;
    case STYLE_SYNTAX:
      s = p;
      while (*s != 0)
        {
          if (isspace (*s) && output_x >= 60)
            {
              ++s;
              write_nl ();
            }
          d = syntax;
          while (*s != 0 && *s != '<' && *s != '|' && *s != '[' && *s != ']'
                 && !(ISSYNTAXARG (s[0]) && s[1] == 0))
            *d++ = *s++;
          if (d != syntax)
            {
              *d = 0;
              start_hilite (HL_TT);
              format_output (syntax, may_break);
              end_hilite ();
            }

          if (ISSYNTAXARG (s[0]) && s[1] == 0)
            {
              start_hilite (HL_SL);
              format_output (s, may_break);
              end_hilite ();
              ++s;
            }

          if (*s == '[' || *s == ']' || *s == '|')
            {
              syntax[0] = *s++;
              syntax[1] = 0;
              start_hilite (HL_BF);
              format_output (syntax, may_break);
              end_hilite ();
            }

          if (*s == '<')
            {
              if (mode != 'T')
                ++s;
              d = syntax;
              while (*s != 0 && *s != '>')
                *d++ = *s++;
              if (*s == '>')
                {
                  if (mode == 'T')
                    *d++ = *s;
                  ++s;
                }
              *d = 0;
              start_hilite (HL_SL);
              format_output (syntax, may_break);
              end_hilite ();
            }
        }
      break;
    default:
      abort ();
    }
}


void format_spaces (int n, enum style style, int may_break)
{
  uchar *p;

  p = alloca (n + 1);
  memset (p, ' ', n);
  p[n] = 0;
  format_string (p, style, may_break);
}


static void start_env (int env, int tindent, int iindent)
{
  if (env_sp >= ENV_STACK_SIZE - 1)
    fatal ("%s:%d: Environment stack overflow", input_fname, line_no);
  ++env_sp;
  env_stack[env_sp].env = env;
  env_stack[env_sp].tmargin = env_stack[env_sp-1].tmargin + tindent;
  env_stack[env_sp].imargin = env_stack[env_sp-1].imargin + iindent;
  env_stack[env_sp].start_line = line_no;
  env_stack[env_sp].counter = 0;
}


static void end_env (int env)
{
  const char *name;

  if (env_stack[env_sp].env != env)
    {
      switch (env_stack[env_sp].env)
        {
        case ENV_DESCRIPTION:
          name = "description";
          break;
        case ENV_ENUMERATE:
          name = "enumerate";
          break;
        case ENV_ITEMIZE:
          name = "itemize";
          break;
        case ENV_LIST:
          name = "list";
          break;
        case ENV_INDENT:
          name = "indent";
          break;
        case ENV_TYPEWRITER:
          name = "typewriter";
          break;
        default:
          fatal ("%s:%d: No environment to end", input_fname, line_no);
          break;
        }
      fatal ("%s:%d: %cend%s expected", input_fname, line_no, escape, name);
    }
  if (out)
    switch (mode)
      {
      case 'H':
        html_end_env ();
        break;
      case 'I':
        ipf_end_env ();
        break;
      case 'L':
        latex_end_env ();
        break;
      }
  --env_sp;
}


static void check_copy (void)
{
  if (out && !copy_flag)
    warning (2, "%s:%d: Text should be inserted here", input_fname, line_no);
  copy_flag = FALSE;
}


static void do_end_env (int env)
{
  end_env (env);
  para_flag = TRUE; copy_flag = FALSE;
  read_line ();
}


static void do_format (const uchar *p)
{
  const uchar *q;
  enum style sty;
  struct word *wp;
  int len;
  uchar word[512];
  uchar flg;

  if (!out)
    {
      flg = 0;
      if (strncmp (p, "bold", 4) == 0 && isspace (p[4]))
        {
          sty = STYLE_BOLD;
          p += 4;
        }
      else if (strncmp (p, "tty", 3) == 0 && isspace (p[3]))
        {
          sty = STYLE_TTY;
          p += 3;
        }
      else if (strncmp (p, "slanted", 7) == 0 && isspace (p[7]))
        {
          sty = STYLE_SLANTED;
          p += 7;
        }
      else if (strncmp (p, "syntax", 6) == 0 && isspace (p[6]))
        {
          sty = STYLE_SYNTAX;
          p += 6;
        }
      else if (strncmp (p, "param", 5) == 0 && isspace (p[5]))
        {
          sty = STYLE_PARAM;
          p += 5;
        }
      else if (strncmp (p, "underline", 9) == 0 && isspace (p[9]))
        {
          sty = STYLE_UNDERLINE;
          p += 9;
        }
      else if (strncmp (p, "emphasize", 9) == 0 && isspace (p[9]))
        {
          sty = STYLE_EMPHASIZE;
          p += 9;
        }
      else if (strncmp (p, "abbrev", 6) == 0 && isspace (p[6]))
        {
          sty = STYLE_NORMAL; flg = WF_ABBREV;
          p += 6;
        }
      else
        fatal ("%s:%d: Invalid style", input_fname, line_no);
      for (;;)
        {
          while (isspace (*p))
            ++p;
          if (*p == 0)
            break;
          len = 0; q = p;
          while (*p != 0 && !isspace (*p))
            word[len++] = *p++;
          word[len] = 0;
          if (sty == STYLE_NORMAL && flg == WF_ABBREV)
            {
              if (len < 2 || word[len-1] != '.')
                fatal ("%s:%d: Abbreviation must end with a period",
                       input_fname, line_no);
              word[len-1] = 0;
            }
          wp = word_add (word);
          if (sty != STYLE_NORMAL)
            wp->style = sty;
          else
            wp->flags |= flg;
        }
    }
  read_line ();
}


static void do_special (const uchar *p)
{
  const uchar *q;
  char m;
  struct word *wp;
  int len;
  uchar word[512], *repl;

  if (!out)
    {
      if (strncmp (p, "text", 4) == 0 && isspace (p[4]))
        {
          m = 'T'; p += 4;
        }
      else if (strncmp (p, "ipf", 3) == 0 && isspace (p[3]))
        {
          m = 'I'; p += 3;
        }
      else if (strncmp (p, "latex", 5) == 0 && isspace (p[5]))
        {
          m = 'L'; p += 5;
        }
      else if (strncmp (p, "html", 4) == 0 && isspace (p[4]))
        {
          m = 'H'; p += 4;
        }
      else
        fatal ("%s:%d: Invalid special mode", input_fname, line_no);
      while (isspace (*p))
        ++p;
      if (*p == 0)
        fatal ("%s:%d: Missing word", input_fname, line_no);
      len = 0; q = p;
      while (*p != 0 && !isspace (*p))
        word[len++] = *p++;
      word[len] = 0;
      while (isspace (*p))
        ++p;
      if (*p == 0)
        fatal ("%s:%d: Missing replacement", input_fname, line_no);
      repl = xstrdup (p);
      wp = word_add (word);
      if (wp->special == NULL)
        {
          wp->special = xmalloc (sizeof (struct special));
          wp->special->text = NULL;
          wp->special->ipf = NULL;
          wp->special->latex = NULL;
          wp->special->html = NULL;
        }
      switch (m)
        {
        case 'H':
          wp->special->html = repl; break;
        case 'T':
          wp->special->text = repl; break;
        case 'I':
          wp->special->ipf = repl; break;
        case 'L':
          wp->special->latex = repl; break;
        default:
          abort ();
        }
    }
  read_line ();
}


static void do_replace (const uchar *p)
{
  const uchar *q;
  struct word *wp;
  int len;
  uchar word[512], *repl;

  if (!out)
    {
      len = 0; q = p;
      while (*p != 0 && !isspace (*p))
        word[len++] = *p++;
      word[len] = 0;
      while (isspace (*p))
        ++p;
      if (*p == 0)
        fatal ("%s:%d: Missing replacement", input_fname, line_no);
      repl = xstrdup (p);
      wp = word_add (word);
      wp->repl = repl;
    }
  read_line ();
}


static void do_set (const uchar *p)
{
  int len, value;
  uchar word[512];

  len = 0;
  if (!isalpha (*p))
    fatal ("%s:%d: Invalid variable name", input_fname, line_no);
  word[len++] = *p++;
  while (isalnum (*p) || *p == '_')
    word[len++] = *p++;
  word[len] = 0;
  if (!isspace (*p))
    fatal ("%s:%d: Invalid variable name ", input_fname, line_no);
  while (isspace (*p))
    ++p;
  if (strcmp (p, "false") == 0)
    value = FALSE;
  else if (strcmp (p, "true") == 0)
    value = TRUE;
  else
    fatal ("%s:%d: Invalid value ", input_fname, line_no);
  cond_set (word, value);
  read_line ();
}


static void do_language (const uchar *p)
{
  if (!out)
    {
      if (strcmp (p, "english") == 0)
        language = LANG_ENGLISH;
      else if (strcmp (p, "german") == 0)
        language = LANG_GERMAN;
      else
        fatal ("%s:%d: Invalid language", input_fname, line_no);
    }
  read_line ();
}


static void do_syntax (const uchar *p)
{
  if (strcmp (p, "emx") == 0)
    language = SYNTAX_EMX;
  else if (strcmp (p, "dvidrv") == 0)
    language = SYNTAX_DVIDRV;
  else
    fatal ("%s:%d: Invalid syntax style", input_fname, line_no);
  read_line ();
}


static void new_section (int level, unsigned flags)
{
  struct toc *tp;
  int i;
  uchar *d;

  tp = xmalloc (sizeof (*tp));
  tp->next = NULL;
  tp->level = level;
  tp->ref = ref_no++;
  tp->print = FALSE;
  tp->global = FALSE;
  if (flags & HF_UNNUMBERED)
    tp->number = NULL;
  else
    {
      ++section_numbers[level-1];
      for (i = level; i < SECTION_LEVELS; ++i)
        section_numbers[i] = 0;
      d = output;
      for (i = 0; i < level; ++i)
        {
          if (i != 0)
            *d++ = '.';
          d += sprintf (d, "%d", section_numbers[i]);
        }
      i = strlen (output);
      if (i > toc_indent)
        toc_indent = i;
      tp->number = xstrdup (output);
    }
  tp->title = NULL;
  tp->flags = flags;
  *toc_add = tp;
  toc_add = &tp->next;
  toc_ptr = tp;
}


static void do_heading_def (const uchar *p)
{
  if (tg_level > 0)
    {
      new_section (tg_level, tg_flags);
      toc_ptr->print = TRUE;
      toc_ptr->title = xstrdup (p);
    }
}

static void do_heading_out (const uchar *p)
{
  check_copy ();
  if (tg_level > 0)
    {
      toc_ptr = (toc_ptr == NULL ? toc_head : toc_ptr->next);
      assert (toc_ptr != NULL);
      assert (strcmp (p, toc_ptr->title) == 0);
      if (toc_ptr->number == NULL)
        output[0] = 0;
      else
        {
          strcpy (output, toc_ptr->number);
          strcat (output, " ");
        }
      switch (mode)
        {
        case 'H':
          html_heading1 (toc_ptr->ref);
          break;
        case 'I':
          ipf_heading1 (toc_ptr->level, toc_ptr->ref, toc_ptr->global,
                        tg_flags);
          break;
        case 'L':
          latex_heading1 ();
          break;
        case 'T':
          text_heading1 ();
          break;
        }
      strcat (output, p);
    }
  else
    strcpy (output, p);
  switch (mode)
    {
    case 'H':
      html_heading2 (output);
      break;
    case 'I':
      ipf_heading2 (output);
      break;
    case 'L':
      latex_heading2 (p);
      break;
    case 'T':
      text_heading2 (output);
      break;
    }
  para_flag = TRUE;
}


static void do_toc_out (const uchar *p)
{
  struct toc *tp;
  int i, len;

  if (out && para_flag)
    switch (mode)
      {
      case 'T':
        write_nl ();
        break;
      }
  para_flag = FALSE;
  if (toc_head != NULL)
    switch (mode)
      {
      case 'H':
        html_toc_start ();
        break;
      case 'I':
        ipf_toc_start ();
        break;
      }
  for (tp = toc_head; tp != NULL; tp = tp->next)
    if (tp->print)
      {
        if (tp->number == NULL)
          len = 0;
        else
          {
            len = strlen (tp->number);
            memcpy (output, tp->number, len);
          }
        i = toc_indent + 2 * tp->level - len;
        memset (output + len, ' ', i);
        output[len+i] = 0;
        switch (mode)
          {
          case 'H':
            html_toc_line (output, tp);
            break;
          case 'I':
            ipf_toc_line (output, tp);
            break;
          case 'T':
            if (!(tp->flags & HF_UNNUMBERED))
              text_toc_line (output, tp);
            break;
          }
      }
  if (mode == 'I' && toc_head != NULL)
    ipf_toc_end ();
  copy_flag = TRUE;
}


static void do_description (void)
{
  check_copy ();
  para_flag = FALSE;
  start_env (ENV_DESCRIPTION, 8, IPF_DESCRIPTION_INDENT);
  if (out)
    switch (mode)
      {
      case 'H':
        html_description ();
        break;
      case 'I':
        ipf_description ();
        break;
      case 'L':
        latex_description ();
        break;
      }
  read_line ();
}


static void do_enumerate (void)
{
  check_copy ();
  para_flag = FALSE;
  start_env (ENV_ENUMERATE, 4, 3);
  if (out)
    switch (mode)
      {
      case 'H':
        html_enumerate ();
        break;
      case 'I':
        ipf_enumerate ();
        break;
      case 'L':
        latex_enumerate ();
        break;
      }
  read_line ();
}


static void do_itemize (void)
{
  check_copy ();
  para_flag = FALSE;
  start_env (ENV_ITEMIZE, 2, 2);
  if (out)
    switch (mode)
      {
      case 'H':
        html_itemize ();
        break;
      case 'I':
        ipf_itemize ();
        break;
      case 'L':
        latex_itemize ();
        break;
      }
  read_line ();
}


static void do_indent (int env)
{
  check_copy ();
  start_env (env, 4, 4);
  ipf_env_margin (env_sp);
  if (out)
    switch (mode)
      {
      case 'H':
        html_indent ();
        break;
      case 'L':
        latex_indent ();
        break;
      }
  read_line ();
}


static void do_list (void)
{
  check_copy ();
  start_env (ENV_LIST, 4, 4);
  if (out)
    switch (mode)
      {
      case 'H':
        html_list ();
        break;
      case 'L':
        latex_description ();
        break;
      }
  read_line ();
}


static void do_verbatim (enum tag tag_end)
{
  int start_line, tmargin;
  const uchar *p;

  check_copy ();
  para_flag = FALSE;
  start_line = line_no;
  tmargin = env_stack[env_sp].tmargin;
  if (out)
    switch (mode)
      {
      case 'H':
        html_verbatim_start (tag_end);
        break;
      case 'I':
        ipf_verbatim_start (tag_end);
        break;
      case 'L':
        latex_verbatim_start (tag_end);
        break;
      case 'T':
        text_verbatim_start (tag_end, &tmargin);
        break;
      }
  for (;;)
    {
      read_line ();
      if (end_of_file)
        switch (tag_end)
          {
          case TAG_ENDEXAMPLE:
            fatal ("%s:%d: Missing %cendexample",
                   input_fname, start_line, escape);
            break;
          case TAG_ENDHEADERS:
            fatal ("%s:%d: Missing %cendheaders",
                   input_fname, start_line, escape);
            break;
          case TAG_ENDSAMPLECODE:
            fatal ("%s:%d: Missing %cendsamplecode",
                   input_fname, start_line, escape);
            break;
          case TAG_ENDVERBATIM:
            fatal ("%s:%d: Missing %cendverbatim",
                   input_fname, start_line, escape);
            break;
          default:
            abort ();
          }
      p = input;
      if (parse_tag (&p) && tg_tag == tag_end)
        break;
      if (out)
        {
          switch (mode)
            {
            case 'H':
              html_verbatim_line ();
              break;
            case 'I':
              ipf_verbatim_line ();
              break;
            case 'L':
            case 'T':
              text_verbatim_line (tag_end, tmargin, compat);
              break;
            }
        }
    }
  if (out)
    switch (mode)
      {
      case 'H':
        html_verbatim_end (tag_end);
        break;
      case 'I':
        ipf_verbatim_end (tag_end);
        break;
      case 'L':
        latex_verbatim_end (tag_end);
        break;
      case 'T':
        para_flag = TRUE;
        break;
      }
  read_line ();
}


static void do_table (const uchar *p)
{
  int start_line, tmargin, wn;
  int do_indent;
  uchar word[512], *d;
  int widths[20];
  char *tmp;
  long n;

  check_copy ();
  para_flag = FALSE;
  start_line = line_no;
  tmargin = env_stack[env_sp].tmargin;
  do_indent = FALSE; wn = 0;
  while (*p != 0)
    {
      d = word;
      while (*p != 0 && !isspace (*p))
        *d++ = *p++;
      *d = 0;
      if (strcmp (word, "indent") == 0)
        do_indent = TRUE;
      else if (isdigit (word[0]))
        {
          errno = 0;
          n = strtol (word, &tmp, 10);
          if (errno != 0 || *tmp != 0)
            fatal ("%s:%d: Invalid %ctable width",
                   input_fname, line_no, escape);
          widths[wn++] = (int)n;
        }
      else
        fatal ("%s:%d: Invalid %ctable attribute",
               input_fname, line_no, escape);
      while (isspace (*p))
        ++p;
    }

  if (out)
    switch (mode)
      {
      case 'H':
        fatal ("%ctable not yet implemented for HTML", escape);
      case 'I':
        ipf_table_start (do_indent, widths, wn);
        break;
      case 'L':
        fatal ("%ctable not yet implemented for LaTeX", escape);
      case 'T':
        text_table_start (do_indent, &tmargin);
        break;
      }
  for (;;)
    {
      read_line ();
      if (end_of_file)
        fatal ("%s:%d: Missing %cendtable", input_fname, start_line, escape);
      p = input;
      if (parse_tag (&p))
        {
          if (tg_tag == TAG_ENDTABLE)
            break;
          fatal ("%s%d: %cendtable expected", input_fname, line_no, escape);
        }
      if (out)
        {
          p = input;
          while (isspace (*p))
            ++p;
          switch (mode)
            {
            case 'I':
              ipf_table_line (p, wn);
              break;
            case 'T':
              text_table_line (p, tmargin);
              break;
            }
        }
    }
  if (out)
    switch (mode)
      {
      case 'I':
        ipf_table_end (do_indent);
        break;
      case 'T':
        para_flag = TRUE;
        break;
      }
  read_line ();
}


static void do_ipf (void)
{
  int start_line;
  const uchar *p;

  start_line = line_no;
  for (;;)
    {
      read_line ();
      if (end_of_file)
        fatal ("%s:%d: Missing %cendipf", input_fname, start_line, escape);
      p = input;
      if (parse_tag (&p) && tg_tag == TAG_ENDIPF)
        break;
      if (out && mode == 'I' && output_flag)
        {
          fprintf (output_file, "%s\n", input);
          output_x = 0; ++output_line_no;
        }
    }
  read_line ();
}


static void do_text (void)
{
  int start_line;
  const uchar *p;

  start_line = line_no;
  for (;;)
    {
      read_line ();
      if (end_of_file)
        fatal ("%s:%d: Missing %cendtext", input_fname, start_line, escape);
      p = input;
      if (parse_tag (&p) && tg_tag == TAG_ENDTEXT)
        break;
      if (out && mode == 'T' && output_flag)
        {
          fprintf (output_file, "%s\n", input);
          output_x = 0; ++output_line_no;
        }
    }
  read_line ();
}


static void do_latex (void)
{
  int start_line;
  const uchar *p;

  start_line = line_no;
  for (;;)
    {
      read_line ();
      if (end_of_file)
        fatal ("%s:%d: Missing %cendlatex", input_fname, start_line, escape);
      p = input;
      if (parse_tag (&p) && tg_tag == TAG_ENDLATEX)
        break;
      if (out && mode == 'L' && output_flag)
        {
          fprintf (output_file, "%s\n", input);
          output_x = 0; ++output_line_no;
        }
    }
  read_line ();
}


static void do_html (void)
{
  int start_line;
  const uchar *p;

  start_line = line_no;
  for (;;)
    {
      read_line ();
      if (end_of_file)
        fatal ("%s:%d: Missing %cendhtml", input_fname, start_line, escape);
      p = input;
      if (parse_tag (&p) && tg_tag == TAG_ENDHTML)
        break;
      if (out && mode == 'H' && output_flag)
        {
          fprintf (output_file, "%s\n", input);
          output_x = 0; ++output_line_no;
        }
    }
  read_line ();
}


static void do_label (const uchar *p)
{
  if (!out)
    {
      if (toc_ptr == NULL)
        fatal ("%s:%d: Cannot use %clabel before %ch1",
               input_fname, line_no, escape, escape);
      define_label (p, toc_ptr->ref, "Label");
    }
  read_line ();
}


static void do_index (const uchar *p)
{
  if (toc_ptr == NULL)
    fatal ("%s:%d: Cannot use %cindex before %ch1",
           input_fname, line_no, escape, escape);
  switch (mode)
    {
    case 'H':
      html_index (toc_ptr, p, tg_level);
      break;
    case 'I':
      ipf_index (p);
      break;
    case 'L':
      latex_index (p, tg_level);
      break;
    }
  read_line ();
}


static void do_keyword (const uchar *p)
{
  switch (mode)
    {
    case 'K':
      keywords_keyword (p);
      break;
    }
  read_line ();
}


static void do_item (const uchar *p)
{
  switch (env_stack[env_sp].env)
    {
    case ENV_DESCRIPTION:
      para_flag = FALSE;
      if (out)
        switch (mode)
          {
          case 'H':
            html_description_item (p);
            break;
          case 'I':
            ipf_description_item (p);
            break;
          case 'L':
            latex_description_item (p);
            break;
          case 'T':
            text_description_item (p);
            break;
          }
      break;
    case ENV_ENUMERATE:
      para_flag = FALSE;
      if (*p != 0)
        fatal ("%s:%d: %citem of %cenumerate doesn't take an argument",
               input_fname, line_no, escape, escape);
      if (out)
        switch (mode)
          {
          case 'H':
            html_enumerate_item ();
            break;
          case 'I':
            ipf_enumerate_item ();
            break;
          case 'L':
            latex_enumerate_item ();
            break;
          case 'T':
            text_enumerate_item ();
            break;
          }
      break;
    case ENV_ITEMIZE:
      para_flag = FALSE;
      if (*p != 0)
        fatal ("%s:%d: %citem of %citemize doesn't take an argument",
               input_fname, line_no, escape, escape);
      if (out)
        switch (mode)
          {
          case 'H':
            html_itemize_item ();
            break;
          case 'I':
            ipf_itemize_item ();
            break;
          case 'L':
            latex_itemize_item ();
            break;
          case 'T':
            text_itemize_item ();
            break;
          }
      break;
    case ENV_LIST:
      para_flag = FALSE;
      if (out)
        switch (mode)
          {
          case 'H':
            html_list_item (p);
            break;
          case 'I':
            ipf_list_item (p);
            break;
          case 'L':
            latex_list_item (p);
            break;
          case 'T':
            text_list_item (p);
            break;
          }
      break;
    default:
      fatal ("%s:%d: %citem outside environment",
             input_fname, line_no, escape);
    }
  read_line ();
}


static void do_prototype (void)
{
  int start_line, tmargin;
  const uchar *p;
  struct element *ep;

  start_line = line_no;
  tmargin = env_stack[env_sp].tmargin;
  prototype_flag = TRUE;
  if (out)
    {
      make_elements_start ();
      switch (mode)
        {
        case 'H':
          html_prototype_start ();
          break;
        case 'I':
          ipf_prototype_start ();
          break;
        case 'L':
          latex_prototype_start ();
          break;
        case 'T':
          text_prototype_start (compat);
          break;
        }
    }
  for (;;)
    {
      read_line ();
      if (end_of_file)
        fatal ("%s:%d: Missing %cendprototype",
               input_fname, start_line, escape);
      p = input;
      if (parse_tag (&p))
        {
          if (tg_tag == TAG_ENDPROTOTYPE)
            break;
          else
            fatal ("%s:%d: %cendprototype expected",
                   input_fname, line_no, escape);
        }
      if (out)
        {
          make_elements_internal (input);
          ep = add_element ();
          ep->el = EL_BREAK;
          ep->n = 1;
        }
    }
  if (out)
    {
      make_elements_end ();
      switch (mode)
        {
        case 'H':
          html_prototype_end (compat);
          break;
        case 'I':
          ipf_prototype_end (compat);
          break;
        case 'L':
          latex_prototype_end (compat);
          break;
        case 'T':
          text_prototype_end ();
          break;
        }
    }
  prototype_flag = FALSE;
  para_flag = TRUE;
  read_line ();
}


static void do_see_also (const uchar *p)
{
  uchar word[512], *d, *o;

  if (out)
    {
      while (isspace (*p))
        ++p;
      o = output; *o = 0;
      switch (mode)
        {
        case 'H':
          html_see_also_start ();
          break;
        case 'I':
          ipf_see_also_start ();
          break;
        case 'L':
          latex_see_also_start ();
          break;
        case 'T':
          text_see_also_start ();
          break;
        }
      while (*p != 0)
        {
          d = word;
          while (*p != 0 && !isspace (*p))
            *d++ = *p++;
          *d = 0;
          while (isspace (*p))
            ++p;
          switch (mode)
            {
            case 'H':
              html_see_also_word (word, p);
              break;
            case 'I':
              ipf_see_also_word (word, p);
              break;
            case 'L':
              latex_see_also_word (word, p);
              break;
            case 'T':
              strcpy (o, word);
              if (*p != 0)
                strcat (o, ", ");
              o = strchr (o, 0);
              break;
            }
        }
      switch (mode)
        {
        case 'T':
          text_see_also_end (output);
          break;
        }
      para_flag = TRUE;
    }
  read_line ();
}


static void do_param (const uchar *p)
{
  uchar word[512], *d;
  struct word *wp;

  if (out)
    {
      while (*p != 0)
        {
          d = word;
          while (*p != 0 && !isspace (*p))
            *d++ = *p++;
          *d = 0;
          while (isspace (*p))
            ++p;
          wp = word_add (word);
          local_add (wp);
          wp->style = STYLE_PARAM;
        }
    }
  read_line ();
}


static void do_sample_file (const uchar *p)
{
  if (out)
    {
      while (isspace (*p))
        ++p;
      switch (mode)
        {
        case 'H':
          html_sample_file (p);
          break;
        case 'I':
          ipf_sample_file (p);
          break;
        case 'L':
          latex_sample_file (p);
          break;
        case 'T':
          text_sample_file (p);
          break;
        }
    }
  read_line ();
}


static void do_libref_section (const uchar *text)
{
  if (out)
    {
      switch (mode)
        {
        case 'H':
          html_libref_section (text);
          break;
        case 'I':
          ipf_libref_section (text);
          break;
        case 'L':
          latex_libref_section (text);
          break;
        case 'T':
          text_libref_section (text);
          break;
        }
    }
  read_line ();
}


static void do_function (const uchar *p)
{
  uchar word[512], *d, *o;
  struct word *wp;

  local_end ();
  local_begin ();
  if (out)
    {
      toc_ptr = (toc_ptr == NULL ? toc_head : toc_ptr->next);
      assert (toc_ptr != NULL);
      switch (mode)
        {
        case 'H':
          html_function_start (toc_ptr);
          break;
        case 'I':
          ipf_function_start (toc_ptr);
          break;
        case 'L':
          latex_function_start (toc_ptr);
          break;
        case 'T':
          text_function ();
          break;
        }
      para_flag = FALSE;
    }
  else
    new_section (2, 0);

  o = output; *o = 0; function_count = 0;
  while (isspace (*p))
    ++p;
  while (*p != 0)
    {
      ++function_count;
      d = word;
      while (*p != 0 && !isspace (*p))
        *d++ = *p++;
      *d = 0;
      while (isspace (*p))
        ++p;
      switch (mode)
        {
        case 'H':
          html_function_function (toc_ptr, word);
          break;
        case 'I':
          ipf_function_function (word);
          break;
        case 'L':
          latex_function_function (word);
          break;
        }
      if (!out)
        {
          wp = define_label (word, toc_ptr->ref, "Function");
          /* Don't set format of variables, timezone! */
          if (strstr (word, "()") != NULL)
            wp->style = STYLE_TTY;
          if (o != output)
            {
              if (*p == 0)
                strcpy (o, " and ");
              else
                strcpy (o, ", ");
            }
          strcat (o, word);
          o = strchr (o, 0);
          switch (mode)
            {
            case 'K':
              write_keyword (word);
              break;
            }
        }
    }
  if (!out)
    toc_ptr->title = xstrdup (output);
  read_line ();
}


static void do_compat (const uchar *p)
{
  while (isspace (*p))
    ++p;
  strcpy (compat, p);
  read_line ();
}


static void do_tag (const uchar *p)
{
  switch (tg_tag)
    {
    case TAG_HEADING:
      local_end ();
      if (out)
        do_heading_out (p);
      else
        {
          if (env_sp != 0)
            nonfatal ("%s:%d: Heading within environment",
                      input_fname, line_no);
          do_heading_def (p);
        }
      read_line ();
      break;

    case TAG_TOC:
      if (out)
        do_toc_out (p);
      read_line ();
      break;

    case TAG_HTMLFRAGMENT:
      if (out && mode == 'H')
        html_fragment (p);
      read_line ();
      break;

    case TAG_IPFMINITOC:
      if (out && mode == 'I')
        ipf_minitoc (toc_ptr);
      read_line ();
      break;

    case TAG_HTMLMINITOC:
      if (out && mode == 'H')
        html_minitoc (toc_ptr);
      read_line ();
      break;

    case TAG_MINITOC:
      if (out)
        switch (mode)
          {
          case 'H':
            html_minitoc (toc_ptr);
            break;
          case 'I':
            ipf_minitoc (toc_ptr);
            break;
          }
      read_line ();
      break;

    case TAG_TITLE:
      if (!out)
        title = xstrdup (p);
      read_line ();
      break;

    case TAG_LANGUAGE:
      do_language (p);
      break;

    case TAG_SYNTAX:
      do_syntax (p);
      break;

    case TAG_ITEM:
      do_item (p);
      break;

    case TAG_IPF:
      do_ipf ();
      break;

    case TAG_TEXT:
      do_text ();
      break;

    case TAG_LATEX:
      do_latex ();
      break;

    case TAG_HTML:
      do_html ();
      break;

    case TAG_LABEL:
      do_label (p);
      break;

    case TAG_INDEX:
      do_index (p);
      break;

    case TAG_KEYWORD:
      do_keyword (p);
      break;

    case TAG_FORMAT:
      do_format (p);
      break;

    case TAG_SPECIAL:
      do_special (p);
      break;

    case TAG_REPLACE:
      do_replace (p);
      break;

    case TAG_SET:
      do_set (p);
      break;

    case TAG_DESCRIPTION:
      do_description ();
      break;

    case TAG_ENDDESCRIPTION:
      do_end_env (ENV_DESCRIPTION);
      break;

    case TAG_ENUMERATE:
      do_enumerate ();
      break;

    case TAG_ENDENUMERATE:
      do_end_env (ENV_ENUMERATE);
      break;

    case TAG_ITEMIZE:
      do_itemize ();
      break;

    case TAG_ENDITEMIZE:
      do_end_env (ENV_ITEMIZE);
      break;

    case TAG_LIST:
      do_list ();
      break;

    case TAG_ENDLIST:
      do_end_env (ENV_LIST);
      break;

    case TAG_INDENT:
      do_indent (ENV_INDENT);
      break;

    case TAG_ENDINDENT:
      do_end_env (ENV_INDENT);
      break;

    case TAG_TYPEWRITER:
      do_indent (ENV_TYPEWRITER);
      break;

    case TAG_ENDTYPEWRITER:
      do_end_env (ENV_TYPEWRITER);
      break;

    case TAG_EXAMPLE:
      do_verbatim (TAG_ENDEXAMPLE);
      break;

    case TAG_SAMPLECODE:
      do_verbatim (TAG_ENDSAMPLECODE);
      break;

    case TAG_HEADERS:
      do_verbatim (TAG_ENDHEADERS);
      break;

    case TAG_PROTOTYPE:
      do_prototype ();
      break;

    case TAG_VERBATIM:
      do_verbatim (TAG_ENDVERBATIM);
      break;

    case TAG_TABLE:
      do_table (p);
      break;

    case TAG_SEEALSO:
      do_see_also (p);
      break;

    case TAG_FUNCTION:
      do_function (p);
      break;

    case TAG_PARAM:
      do_param (p);
      break;

    case TAG_COMPAT:
      do_compat (p);
      break;

    case TAG_SAMPLEFILE:
      do_sample_file (p);
      break;

    case TAG_RESTRICTIONS:
      do_libref_section ("Restrictions");
      break;

    case TAG_IMPLEMENTATION:
      do_libref_section ("Implementation-defined behavior");
      break;

    case TAG_BUGS:
      do_libref_section ("Bugs");
      break;

    case TAG_ERRORS:
      do_libref_section ("Errors");
      break;

    case TAG_HINTS:
      do_libref_section ("Hints");
      break;

    case TAG_RETURNVALUE:
      do_libref_section ("Return value");
      break;

    default:
      fatal ("%s:%d: Unexpected tag", input_fname, line_no);
    }
}


static void do_copy (void)
{
  size_t len;
  const uchar *p;
  int space, col1_warn;

  line_len = 0; space = FALSE; col1_warn = FALSE;
  do
    {
      if (out)
        {
          p = input;
          while (isspace (*p))
            ++p;
          if (p == input && !col1_warn)
            {
              col1_warn = TRUE;
              warning (1, "%s:%d: Text starting in first column",
                       input_fname, line_no);
            }
          len = strlen (p);
          while (len > 0 && isspace (p[len-1]))
            --len;
          if (len > 0)
            {
              if (space)
                line_add (" ", 1);
              line_add (p, len);
            }
          space = TRUE;
        }
      read_line ();
      p = input;
    } while (!end_of_file && input[0] != 0 && !parse_tag (&p));
  if (out)
    {
      make_elements (line);
      switch (mode)
        {
        case 'H':
          html_copy ();
          break;
        case 'I':
          ipf_copy ();
          break;
        case 'L':
          latex_copy ();
          break;
        case 'T':
          text_copy ();
          break;
        }
    }
  para_flag = TRUE; copy_flag = TRUE;
}


static void do_empty_line (void)
{
  read_line ();
}


void start_index (void)
{
  const char *text = "Index";
  local_end ();
  tg_level = 1; tg_flags = 0;
  if (out)
    do_heading_out (text);
  else
    do_heading_def (text);
}


static void init (void)
{
  word_top = wt_new (997);
}


void init_file (void)
{
  int i;

  for (i = 0; i < SECTION_LEVELS; ++i)
    section_numbers[i] = 0;
  toc_ptr = NULL;

  if (out)
    switch (mode)
      {
      case 'H':
        html_start ();
        break;
      case 'I':
        ipf_start ();
        break;
      case 'L':
        latex_start ();
        break;
      }

  para_flag = FALSE;
  env_sp = 0;
  env_stack[env_sp].env = ENV_NONE;
  env_stack[env_sp].tmargin = 0;
  env_stack[env_sp].imargin = 1;
  env_stack[env_sp].start_line = 0;
  local_sp = 0;
  hl_ipf = 0;
  hl_ipf_no = 0;
  hl_sp = 0;
  hl_stack[hl_sp] = 0;
  output_x = 0; output_line_no = 1;
  compat[0] = 0;
  syntax_style = SYNTAX_EMX;
  copy_flag = TRUE;
}


static void end_file (void)
{
  switch (mode)
    {
    case 'H':
      html_end ();
      break;
    case 'I':
      ipf_end ();
      break;
    case 'L':
      latex_end ();
      break;
    }
  if (env_sp != 0)
    fatal ("%s:%d: Environment not terminated",
           input_fname, env_stack[env_sp].start_line);
}


static void read_file (const char *name)
{
  const uchar *p;

  if (xref_fname != NULL)
    read_xref (xref_fname, toc_head);
  open_input (name);
  init_file ();
  read_line ();
  while (!end_of_file)
    {
      p = input;
      if (input[0] == 0)
        do_empty_line ();
      else if (parse_tag (&p))
        do_tag (p);
      else
        do_copy ();
    }
  end_file ();
  fclose (input_file);
}


static void cleanup (void)
{
  if (output_fname != NULL && output_file != stdout && output_file != NULL)
    {
      fclose (output_file);
      output_file = NULL;
      remove (output_fname);
      output_fname = NULL;
    }
}


static int opt_number (void)
{
  int result;
  char *tmp;

  errno = 0;
  result = (int)strtol (optarg, &tmp, 0);
  if (errno != 0 || *tmp != 0)
    usage ();
  return result;
}

static enum enc opt_encoding (void)
{
  if (strcmp (optarg, "cp850") == 0)
    return ENC_CP850;
  else if (strcmp (optarg, "iso8859-1") == 0)
    return ENC_ISO8859_1;
  else
    fatal ("Invalid encoding: %s", optarg);
}

int main (int argc, char *argv[])
{
  int c, i;

  init ();

  while ((c = getopt (argc, argv, ":?HIKLMTab:ce:fgh:i:j:n:o:rw:x:")) != EOF)
    switch (c)
      {
      case '?':
        usage ();
      case 'H':
      case 'I':
      case 'K':
      case 'L':
      case 'M':
      case 'T':
        mode = (char)c;
        break;
      case 'a':
        opt_a = TRUE;
        break;
      case 'b':
        opt_b = opt_number ();
        if (opt_b < 0 || opt_b > 1)
          fatal ("Invalid argument for -b (line breaking algorithm)");
        break;
      case 'c':
        opt_c = TRUE;
        break;
      case 'e':
        if (strlen (optarg) == 1 && !isdigit ((uchar)*optarg))
          escape = *optarg;
        else
          {
            i = opt_number ();
            if (i < 1 || i > 255)
              fatal ("Invalid argument for -e (escape character)");
            escape = (uchar)i;
          }
        break;
      case 'f':
        opt_f = TRUE;
        break;
      case 'g':
        opt_g = TRUE;
        break;
      case 'h':
        hyphenation_fname = optarg;
        break;
      case 'i':
        input_encoding = opt_encoding ();
        break;
      case 'j':
        output_encoding = opt_encoding ();
        break;
      case 'n':
        ref_no = opt_number ();
        break;
      case 'o':
        output_fname = optarg;
        break;
      case 'r':
        opt_r = TRUE;
        break;
      case 'w':
        opt_w = opt_number ();
        break;
      case 'x':
        xref_fname = optarg;
        break;
      case ':':
        fatal ("Missing argument for option -%c", optopt);
      default:
        fatal ("Invalid option: -%c", optopt);
      }

  if (mode == 0)
    usage ();
  if (mode != 'I' && (opt_c || opt_g))
    usage ();
  if (mode != 'I' && mode != 'H' && xref_fname != NULL)
    usage ();
  if ((mode == 'H' || mode == 'I') && output_encoding != ENC_DEFAULT)
    warning (0, "Output encoding ignored for -H and -I");

  argv += optind;
  argc -= optind;

  if (argc != 1 && mode != 'M')
    usage ();

  atexit (cleanup);

  switch (mode)
    {
    case 'T':
      max_width = 79;
      output_flag = TRUE;
      if (hyphenation_fname != NULL)
        text_hyphenation (hyphenation_fname);
      if (output_encoding == ENC_DEFAULT)
        output_encoding = input_encoding;
      break;
    case 'I':
      max_width = 250;
      output_flag = !opt_g;
      output_encoding = ENC_CP850;
      break;
    case 'H':
      max_width = 4096;
      output_flag = TRUE;
      output_encoding = ENC_ISO8859_1;
      break;
    case 'L':
      max_width = 4096;
      output_flag = TRUE;
      if (output_encoding == ENC_DEFAULT)
        output_encoding = input_encoding;
      break;
    case 'K':
    case 'M':
      output_flag = FALSE;
      if (output_encoding == ENC_DEFAULT)
        output_encoding = input_encoding;
      break;
    default:
      abort ();
    }

  if (output_fname == NULL)
    {
      output_fname = "<stdout>";
      output_file = stdout;
    }
  else
    {
      if (opt_r)
        {
          chmod (output_fname, 0666);
          remove (output_fname);
        }
      output_file = fopen (output_fname, "wt");
      if (output_file == NULL)
        {
          perror (output_fname);
          return 1;
        }
    }

  switch (mode)
    {
    case 'H':
    case 'I':
    case 'T':
    case 'L':
      out = FALSE;
      read_file (argv[0]);
      out = TRUE;
      read_file (argv[0]);
      break;
    case 'K':
      keywords_start (argv[0]);
      out = FALSE;
      read_file (argv[0]);
      break;
    case 'M':
      out = FALSE;
      for (i = 0; i < argc; ++i)
        make_global (argv[i]);
      out = TRUE;
      for (i = 0; i < argc; ++i)
        make_global (argv[i]);
      break;
    }

  if (fflush (output_file) != 0 || fclose (output_file) != 0)
    {
      perror (output_fname);
      return 1;
    }
  output_file = NULL;

  if (opt_r)
    chmod (output_fname, 0444);

  return history;
}
