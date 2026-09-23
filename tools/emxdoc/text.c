/*!
 * @file text.c
 * @brief Text output.
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
#include <malloc.h>
#include "emxdoc.h"
#include "text.h"
#include "lb.h"

/*!
 * @brief Target output width for text mode.
 */
static int text_width = 70;
/*!
 * @brief Current formatting width.
 */
static int format_width;
/*!
 * @brief Current formatting margin.
 */
static int format_margin;
/*!
 * @brief Current line-breaking state.
 */
static struct lb *lb;
/*!
 * @brief Hyphenation dictionary or NULL.
 */
static struct lbh *lbh = NULL;

/*!
 * @brief Emit spaces so the output column reaches @p margin.
 *
 * @param[in] margin Target column.
 */
static void text_indent (int margin)
{
  uchar *s;
  int i;

  i = margin - output_x;
  if (i > 0)
    {
      s = alloca (i);
      memset (s, ' ', i);
      write_nstring (s, i);
    }
}


/*!
 * @brief Finish the current paragraph if one is pending.
 */
static void text_para (void)
{
  if (para_flag)
    {
      para_flag = FALSE;
      write_nl ();
    }
}


/*!
 * @brief Test whether a string consists only of spaces.
 *
 * @param[in] p String to test. Not NULL.
 *
 * @return TRUE if all characters are spaces, FALSE otherwise.
 *
 * @retval TRUE   The string consists only of spaces.
 * @retval FALSE  Otherwise.
 */
static int isblank (const uchar *p)
{
  while (*p != 0)
    if (*p++ != ' ')
      return FALSE;
  return TRUE;
}


/*!
 * @brief Write a string with the current formatting.
 *
 * @param[in] p         String. Not NULL.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
void text_output (const uchar *p, int may_break)
{
  if (opt_b == 0)
    {
      if (may_break && output_x + strlen (p) > format_width)
        write_nl ();
      text_indent (format_margin);
      write_string (p);
    }
  else
    {
      int rc;

      if (may_break && isblank (p))
        rc = lb_glue (lb, strlen (p), NULL);
      else
        rc = lb_word (lb, strlen (p), p, NULL);
      if (rc != 0)
        fatal ("lb_glue or lb_word failed, rc=%d", rc);
    }
}


/*!
 * @brief Render the current element list.
 *
 * @param[in] margin   Left margin.
 * @param[in] width    Right margin.
 * @param[in] newline  Non-zero to finish the paragraph with a newline.
 *
 * @return Number of output lines generated.
 */
static int text_elements (int margin, int width, int newline)
{
  const struct element *ep, *ep2;
  int i, spaces, line, rc;
  enum style style_stack[STYLE_STACK_SIZE];
  int style_sp;
  enum style style = STYLE_NORMAL;

  style_sp = 0;
  style_stack[style_sp] = style;
  line = output_line_no; spaces = 0;
  format_width = width; format_margin = margin;
  if (opt_b != 0)
    {
      rc = lb_init (&lb, margin, width);
      if (rc != 0)
        fatal ("lb_init failed, rc=%d", rc);
      if (output_x > margin)
        lb_first_lmargin (lb, output_x);
    }

  for (ep = elements; ep->el != EL_END; ++ep)
    switch (ep->el)
      {
      case EL_WORD:
      case EL_PUNCT:
        if (spaces != 0)
          {
            format_spaces (spaces, STYLE_NORMAL, TRUE); spaces = 0;
          }
        if (ep->wp->special != NULL && ep->wp->special->text != NULL)
          format_output (ep->wp->special->text, FALSE);
        else if (style_sp == 0 || style_stack[style_sp] == STYLE_NORMAL)
          format_string (ep->wp->str, (ep->wp->style != STYLE_NORMAL
                                       ? ep->wp->style : style), FALSE);
        else
          format_string (ep->wp->str, style_stack[style_sp], FALSE);
        break;
      case EL_SPACE:
        if (opt_b == 0)
          {
            spaces += ep->n;
            i = 0;
            ep2 = ep + 1;
            while (ep2->el != EL_END && ep2->el != EL_SPACE)
              {
                if (ep2->el == EL_WORD || ep2->el == EL_PUNCT)
                  i += strlen (ep2->wp->str);
                ++ep2;
              }
            if (output_x + spaces + i > format_width)
              {
                write_nl ();
                spaces = 0;
              }
          }
        else
          format_spaces (ep->n, STYLE_NORMAL, TRUE);
        break;
      case EL_BREAK:
        if (opt_b == 0)
          {
            if (output_x != 0 || ep->n)
              {
                write_nl (); spaces = 0;
              }
          }
        else
          lb_penalty (lb, -10000);
        break;
      case EL_STYLE:
        if (style_sp + 1 >= STYLE_STACK_SIZE)
          fatal ("%s:%d: Style stack overflow", input_fname, line_no);
        style_stack[++style_sp] = ep->n;
        break;
      case EL_ENDSTYLE:
        if (style_sp == 0)
          fatal ("%s:%d: Style stack underflow", input_fname, line_no);
        --style_sp;
        break;
      default:
        abort ();
      }

  if (opt_b != 0)
    {
      struct lb_node lbn;

      if (lbh != NULL)
        lb_use_hyphenation (lb, lbh);
      rc = lb_format (lb);
      if (rc != 0)
        fatal ("lb_format failed, rc=%d", rc);
      for (;;)
        {
          rc = lb_next (lb, &lbn);
          if (rc != 0)
            fatal ("lb_next failed, rc=%d", rc);
          if (lbn.type == LBN_END)
            break;
          switch (lbn.type)
            {
            case LBN_WORD:
            case LBN_PRE:
            case LBN_POST:
              text_indent (margin);
              write_string (lbn.word);
              break;
            case LBN_GLUE:
              text_indent (margin);
              write_fmt ("%*s", lbn.value, "");
              break;
            case LBN_NEWLINE:
              write_nl ();
              break;
            default:
              abort ();
            }
        }
      lb_exit (&lb);
    }

  if (newline && output_x > margin)
    write_nl ();
  return output_line_no - line;
}


/*!
 * @brief Emit the start of a level-1 heading.
 */
void text_heading1 (void)
{
  env_stack[0].tmargin = 0;
  write_nl ();
  write_nl ();
}


/*!
 * @brief Emit a heading.
 *
 * @param[in,out] s Heading text; overwritten with the underline.
 */
void text_heading2 (uchar *s)
{
  if (tg_level == 0)
    {
      if (para_flag)
        write_nl ();
      write_nl ();
    }
  write_line (s);
  memset (s, tg_underline, strlen (s));
  write_line (s);
}


/*!
 * @brief Begin a "See also" paragraph.
 */
void text_see_also_start (void)
{
  if (para_flag)
    write_nl ();
}


/*!
 * @brief Emit the "See also" list.
 *
 * @param[in] s Comma separated reference list. Not NULL.
 */
void text_see_also_end (const uchar *s)
{
  write_string ("  See also: ");
  make_elements (s);
  text_elements (output_x, text_width, TRUE);
}


/*!
 * @brief Emit a description-list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void text_description_item (const uchar *s)
{
  write_nl ();
  text_para ();
  make_elements (s);
  text_elements (env_stack[env_sp-1].tmargin, text_width, FALSE);
  if (output_x + 2 <= env_stack[env_sp].tmargin)
    write_string ("  ");
  else
    write_nl ();
}


/*!
 * @brief Emit an enumerated-list item.
 */
void text_enumerate_item (void)
{
  write_nl ();
  text_para ();
  text_indent (env_stack[env_sp-1].tmargin);
  write_fmt ("%d.", ++env_stack[env_sp].counter);
}


/*!
 * @brief Emit an itemized-list item.
 */
void text_itemize_item (void)
{
  write_nl ();
  text_para ();
  text_indent (env_stack[env_sp-1].tmargin);
  write_string ("- ");
}


/*!
 * @brief Emit a list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void text_list_item (const uchar *s)
{
  write_nl ();
  text_para ();
  make_elements (s);
  text_elements (env_stack[env_sp-1].tmargin, text_width, TRUE);
  write_nl ();
}


/*!
 * @brief Emit a paragraph of normal text.
 */
void text_copy (void)
{
  if (para_flag)
    write_nl ();
  text_elements (env_stack[env_sp].tmargin, text_width, TRUE);
}


/*!
 * @brief Begin a text verbatim block.
 *
 * @param[in]     tag_end  Tag that terminates the block.
 * @param[in,out] ptmargin Top-margin pointer to adjust. Not NULL.
 */
void text_verbatim_start (enum tag tag_end, int *ptmargin)
{
  switch (tag_end)
    {
    case TAG_ENDHEADERS:
      break;
    case TAG_ENDSAMPLECODE:
      write_nl ();
      write_line ("  Example:");
      write_nl ();
      break;
    case TAG_ENDEXAMPLE:
      *ptmargin += 4;
      write_nl ();
      break;
    default:
      write_nl ();
      break;
    }
}


/*!
 * @brief Append a compatibility note to a line.
 *
 * @param[in,out] dst    Line buffer. Not NULL.
 * @param[in,out] compat Compatibility text. Cleared on output. Not NULL.
 */
static void add_compat (uchar *dst, uchar *compat)
{
  int i, len;

  len = strlen (dst);
  i = 78 - len - strlen (compat) - 2;
  if (i > 0)
    {
      memset (dst + len, ' ', i);
      len += i;
    }
  dst[len++] = '[';
  strcpy (dst+len, compat);
  strcat (dst+len, "]");
  compat[0] = 0;
}


/*!
 * @brief Emit one verbatim line.
 *
 * @param[in]     tag_end Tag that terminates the block.
 * @param[in]     tmargin Top margin.
 * @param[in,out] compat  Compatibility buffer. Not NULL.
 */
void text_verbatim_line (enum tag tag_end, int tmargin, uchar *compat)
{
  uchar buf[512];

  memset (buf, ' ', tmargin);
  strcpy (buf + tmargin, input);
  if (tag_end == TAG_ENDHEADERS && compat[0] != 0)
    add_compat (buf, compat);
  write_line (buf);
}


/*!
 * @brief Emit a function documentation separator.
 */
void text_function (void)
{
  env_stack[0].tmargin = 0;
  write_nl ();
  write_line("---------------------------------------"
             "---------------------------------------");
}


/*!
 * @brief Begin a prototype block.
 *
 * @param[in,out] compat Compatibility buffer. Not NULL.
 */
void text_prototype_start (uchar *compat)
{
  uchar buf[512];

  if (compat[0] != 0)
    {
      buf[0] = 0;
      add_compat (buf, compat);
      write_string (buf);
    }
  write_nl ();
}


/*!
 * @brief End a prototype block.
 */
void text_prototype_end (void)
{
  text_elements (0, 78, TRUE);
  env_stack[0].tmargin = 4;
}


/*!
 * @brief Emit one table-of-contents entry.
 *
 * @param[in] s  Section number text.
 * @param[in] tp Table-of-contents entry.
 */
void text_toc_line (const uchar *s, const struct toc *tp)
{
  write_string (s);
  write_line (tp->title);
}


/*!
 * @brief Begin a text table.
 *
 * @param[in]     do_indent Non-zero to indent the table.
 * @param[in,out] ptmargin  Top-margin pointer to adjust. Not NULL.
 */
void text_table_start (int do_indent, int *ptmargin)
{
  if (do_indent)
    *ptmargin += 4;
  write_nl ();
}


/*!
 * @brief Emit one table line.
 *
 * @param[in] s       Line text. Not NULL.
 * @param[in] tmargin Top margin.
 */
void text_table_line (const uchar *s, int tmargin)
{
  make_elements (s);
  if (text_elements (tmargin, 78, TRUE) > 1)
    fatal ("%s:%d: Table entry too long", input_fname, line_no);
}


/*!
 * @brief Emit a sample-file reference.
 *
 * @param[in] s Sample file name. Not NULL.
 */
void text_sample_file (const uchar *s)
{
  if (para_flag)
    write_nl ();
  write_string ("  Example: See ");
  write_line (s);
}


/*!
 * @brief Emit a library-reference section heading.
 *
 * @param[in] s Section title. Not NULL.
 */
void text_libref_section (const uchar *s)
{
  if (para_flag)
    write_nl ();
  write_fmt ("  %s:", s);
  write_nl ();
  para_flag = TRUE;
}


/*!
 * @brief Load a hyphenation table.
 *
 * @param[in] name File name. Not NULL.
 */
void text_hyphenation (const char *name)
{
  FILE *f;
  char line[128], *p;
  int line_no, rc;

  if (opt_b == 0)
    return;
  rc = lbh_init (&lbh);
  if (rc != 0)
    fatal ("lb_init failed, rc=%d", rc);
  f = fopen (name, "rt");
  if (f == NULL)
    {
      perror (name);
      exit (1);
    }
  line_no = 0;
  while (fgets (line, sizeof (line), f) != NULL)
    {
      ++line_no;
      p = strchr (line, '\n');
      if (p == NULL)
        fatal ("%s:%d: Line too long", name, line_no);
      *p = 0;
      rc = lbh_word (lbh, line);
      if (rc != 0)
        fatal ("lbh_word failed, rc=%d", rc);
    }
  if (ferror (f))
    {
      perror (name);
      exit (1);
    }
  fclose (f);
}
