/*!
 * @file xref.c
 * @brief Manage cross references.
 *
 * Copyright (c) 1993-1999 Eberhard Mattes
 *
 * This file is part of emxdoc.
 *
 * emxdoc is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * emxdoc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with emxdoc; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "emxdoc.h"
#include "xref.h"

#ifndef _MAX_FNAME

/* Quick and dirty implementation of a subset of _splitpath() for
   systems which don't have _splitpath(). */

#include <sys/param.h>
#include <assert.h>
/*!
 * @def _MAX_FNAME
 * @brief Maximum length of a base file name.
 */
#define _MAX_FNAME MAXPATHLEN

/*!
 * @brief Extract the base name of a path.
 *
 * @param[in]  src  Source path. Not NULL.
 * @param[out] drv  Unused; must be NULL.
 * @param[out] dir  Unused; must be NULL.
 * @param[out] base Receives the base name. Not NULL.
 * @param[out] ext  Unused; must be NULL.
 */
static void _splitpath (const char *src, char *drv, char *dir,
			char *base, char *ext)
{
  const char *p, *q;
  size_t n;

  assert (drv == NULL); assert (dir == NULL); assert (ext == NULL);
  assert (src != NULL); assert (base != NULL);

  p = strrchr (src, '/');
  if (p == NULL)
    p = src;
  q = strchr (p, '.');
  if (q == NULL)
    n = strlen (p);
  else
    n = q - p;
  assert (n < _MAX_FNAME);
  memcpy (base, p, n);
  base[n] = 0;
}

#endif

/*!
 * @brief Non-zero until the first keyword has been written.
 */
static int first_keyword_flag = TRUE;
/*!
 * @brief Base name of the current .inf file.
 */
static char inf_name[_MAX_FNAME] = "";


/*!
 * @brief Write one keyword line to the output file.
 *
 * @param[in] p Keyword text. Not NULL.
 */
void write_keyword (const uchar *p)
{
  int len;

  if (*p != 0 && strpbrk (p, " ,") == NULL)
    {
      if (first_keyword_flag)
        {
          if (title != NULL)
            fprintf (output_file, "DESCRIPTION: %s\n", title);
          else
            fprintf (output_file, "DESCRIPTION: %s.inf\n", inf_name);
          first_keyword_flag = FALSE;
        }
      len = strlen (p);
      if (len > 2 && p[len-2] == '(' && p[len-1] == ')')
        len -= 2;
      fprintf (output_file, "(%.*s, view %s ~)\n", len, p, inf_name);
    }
}


/*!
 * @brief Write a global "d" (define) record.
 *
 * @param[in] wp Word being defined. Not NULL.
 */
static void gather_define (struct word *wp)
{
  fprintf (output_file, "d %d %s\n", wp->ref, wp->str);
}


/*!
 * @brief Write a global "r" (reference) record.
 *
 * @param[in] p Referenced label name. Not NULL.
 */
static void gather_reference (const uchar *p)
{
  struct word *wp;

  fprintf (output_file, "r %s\n", p);
  wp = word_add (p);
  wp->ref = -1;
}


/*!
 * @brief Define a document label.
 *
 * @param[in] p   Label name. Not NULL.
 * @param[in] ref Reference number.
 * @param[in] msg Kind of definition, used in diagnostics. Not NULL.
 *
 * @return Pointer to the word representing the label.
 */
struct word *define_label (const uchar *p, int ref, const char *msg)
{
  struct word *wp;

  wp = word_add (p);
  if (wp->ref != 0)
    {
      if (wp->database == NULL || wp->ref != ref)
        fatal ("%s:%d: %s %s multiply defined", input_fname, line_no, msg, p);
    }
  else
    {
      wp->ref = ref;
      if (opt_g)
        gather_define (wp);
    }
  return wp;
}


/*!
 * @brief Resolve a reference to a document label.
 *
 * @param[in] p Label name. Not NULL.
 *
 * @return Pointer to the referenced word, or NULL if unresolved.
 */
struct word *use_reference (const uchar *p)
{
  struct word *wp;

  wp = word_find (p, word_hash (p));
  if ((wp == NULL || wp->ref == 0) && mode != 'T' && mode != 'L')
    {
      if (!opt_g)
        fatal ("%s:%d: Undefined label: %s", input_fname, line_no, p);
      gather_reference (p);
      return NULL;
    }
  else
    return wp;
}


/* Misuse the style field of struct word */

/*!
 * @def STYLE_UNUSED
 * @brief Style value marking a global entry as not yet used.
 */
#define STYLE_UNUSED STYLE_NORMAL
/*!
 * @def STYLE_USED
 * @brief Style value marking a global entry as used.
 */
#define STYLE_USED   STYLE_BOLD

/*!
 * @brief Read or write global cross-reference information for one file.
 *
 * @param[in] name File name. Not NULL.
 */
void make_global (const uchar *name)
{
  struct word *wdb, *wp;
  const uchar *p;
  char *tmp;
  long n;
  char database[_MAX_FNAME];

  open_input (name);
  init_file ();
  wdb = NULL;
  if (!out)
    {
      _splitpath (input_fname, NULL, NULL, database, NULL);
      wdb = word_add (database);
    }
  read_line ();
  while (!end_of_file)
    {
      p = input + 1;
      while (isspace (*p))
        ++p;
      switch (input[0])
        {
        case 'd':
          if (!out)
            {
              errno = 0;
              n = strtol (p, &tmp, 10);
              if (errno != 0 || *tmp != ' ')
                fatal ("%s:%d: Invalid reference number",
                       input_fname, line_no);
              p = tmp;
              while (isspace (*p))
                ++p;
              wp = word_add (p);
              if (wp->ref != 0)
                fatal ("%s:%d: Label %s already defined",
                       input_fname, line_no, p);
              wp->ref = (int)n;
              wp->style = STYLE_UNUSED;
              wp->database = wdb;
            }
          break;
        case 'r':
          if (out)
            {
              wp = word_find (p, word_hash (p));
              if (wp == NULL || wp->ref == 0)
                fatal ("%s:%d: Label %s not defined",
                       input_fname, line_no, p);
              if (wp->style == STYLE_UNUSED)
                {
                  fprintf (output_file, "x %d %s %s\n",
                           wp->ref, wp->database->str, p);
                  wp->style = STYLE_USED;
                }
            }
          break;
        }
      read_line ();
    }
  fclose (input_file);
}


/*!
 * @brief Write one or more keyword lines.
 *
 * @param[in] s Whitespace separated keyword list. Not NULL.
 */
void keywords_keyword (const uchar *s)
{
  uchar word[512], *d;

  while (*s != 0)
    {
      d = word;
      while (*s != 0 && !isspace (*s))
        *d++ = *s++;
      *d = 0;
      write_keyword (word);
      while (isspace (*s))
        ++s;
    }
}


/*!
 * @brief Emit the prologue of a keyword file.
 *
 * @param[in] fname Input file name. Not NULL.
 */
void keywords_start (const char *fname)
{
  _splitpath (fname, NULL, NULL, inf_name, NULL);
  fprintf (output_file, "EXTENSIONS: *\n");
}


/*!
 * @brief Read a cross-reference file.
 *
 * @param[in] fname Cross-reference file name. Not NULL.
 * @param[in] th    Head of the table of contents. Not NULL.
 */
void read_xref (const char *fname, struct toc *th)
{
  struct word *wp;
  uchar database[512], *d;
  const uchar *p;
  char *tmp;
  long n;
  struct toc *tp;

  open_input (fname);
  read_line ();
  while (!end_of_file)
    {
      if (input[0] != 'x')
        fatal ("%s:%d: Syntax error", fname, line_no);
      p = input + 1;
      while (isspace (*p))
        ++p;
      errno = 0;
      n = strtol (p, &tmp, 10);
      if (errno != 0 || *tmp != ' ')
        fatal ("%s:%d: Syntax error", fname, line_no);
      p = tmp;
      while (isspace (*p))
        ++p;
      d = database;
      while (*p != 0 && !isspace (*p))
        *d++ = *p++;
      *d = 0;
      while (isspace (*p))
        ++p;
      if (d == database || *p == 0)
        fatal ("%s:%d: Syntax error", fname, line_no);
      if (out)
        {
          for (tp = th; tp != NULL; tp = tp->next)
            if (tp->ref == (int)n)
              tp->global = TRUE;
        }
      else
        {
          wp = word_add (p);
          wp->database = word_add (database);
          wp->ref = (int)n;
        }
      read_line ();
    }
  fclose (input_file);
}
