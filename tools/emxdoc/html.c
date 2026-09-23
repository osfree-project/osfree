/*!
 * @file html.c
 * @brief HTML output.
 *
 * Copyright (c) 1999 Eberhard Mattes
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
#include <assert.h>
#include "emxdoc.h"
#include "html.h"
#include "xref.h"

/*!
 * @brief Number of index entries collected so far.
 */
static int index_count;
/*!
 * @brief Array of index word pointers being sorted.
 */
static struct word **index_v;
/*!
 * @brief Number of entries currently stored in index_v.
 */
static int index_n;
/*!
 * @brief Current top-level index word.
 */
static struct word *index_wp1;
/*!
 * @brief Case-folding map used while sorting the index.
 */
static const uchar *index_map;

/*!
 * @brief Emit the collected index to the output file.
 */
static void html_write_index (void);

/*!
 * @brief Write a string, escaping HTML metacharacters.
 *
 * @param[in] p         String. Not NULL.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
void html_output (const uchar *p, int may_break)
{
  const uchar *q;

  if (*p == ' ' && output_x >= 60 && may_break)
    {
      write_nl ();
      ++p;
    }
  while ((q = strpbrk (p, "<>& ")) != NULL)
    {
      write_nstring (p, q - p);
      p = q;
      if (p[0] == ' ' && p[1] == ' ')
        while (*p == ' ')
          {
            write_string ("&nbsp;");
            ++p;
          }
      else
        switch (*p++)
          {
          case '<':
            write_string ("&lt;");
            break;
          case '>':
            write_string ("&gt;");
            break;
          case '&':
            write_string ("&amp;");
            break;
          case ' ':
            write_string (" ");
            break;
          default:
            abort ();
            }
    }
  write_string (p);
}

/*!
 * @brief Start a new HTML paragraph.
 */
static void html_para (void)
{
  write_break ();
  write_string ("<P>");
}

/*!
 * @brief Open a hyperlink to a document anchor.
 *
 * @param[in] database Database name, or NULL for the current file.
 * @param[in] ref      Anchor reference number.
 */
static void html_begin_link (const uchar *database, int ref)
{
  write_fmt ("<A HREF=\"%s%s#%d\">",
             database != NULL ? (const char *)database : "",
             database != NULL ? ".html" : "", ref);
}


/*!
 * @brief Close a hyperlink opened by html_begin_link().
 */
static void html_end_link (void)
{
  write_string ("</A>");
}


/*!
 * @brief Render the current element list.
 *
 * @param[in] style Default style to apply.
 */
void html_elements (enum style style)
{
  const struct element *ep;
  enum style style_stack[STYLE_STACK_SIZE];
  int style_sp, ignore_spaces;
  uchar *p;

  style_sp = 0;
  style_stack[style_sp] = style;
  ignore_spaces = FALSE;
  for (ep = elements; ep->el != EL_END; ++ep)
    switch (ep->el)
      {
      case EL_WORD:
      case EL_PUNCT:
        if (ep->n != 0)
          {
            if (ep->wp->database == NULL)
              html_begin_link (NULL, ep->wp->ref);
            else
              html_begin_link (ep->wp->database->str, ep->wp->ref);
          }
        p = ep->wp->str;
        if (ep->wp->special != NULL && ep->wp->special->html != NULL)
          write_string (ep->wp->special->html);
        else if (style_sp == 0 || style_stack[style_sp] == STYLE_NORMAL)
          format_string (p, (ep->wp->style != STYLE_NORMAL
                             ? ep->wp->style : style), FALSE);
        else
          format_string (p, style_stack[style_sp], FALSE);
        if (ep->n != 0)
          html_end_link ();
        break;
      case EL_SPACE:
        if (ignore_spaces)
          ignore_spaces = FALSE;
        else
          format_spaces (ep->n, style_stack[style_sp], TRUE);
        break;
      case EL_BREAK:
        write_line ("<BR>");
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
}

/*!
 * @brief Emit the opening tag for the current highlight.
 */
void html_start_hilite (void)
{
  if (hl_stack[hl_sp] & HL_TT)
    write_string ("<TT>");
  else if (hl_stack[hl_sp] & HL_BF)
    write_string ("<B>");
  else if (hl_stack[hl_sp] & (HL_SL | HL_EM))
    write_string ("<I>");
}


/*!
 * @brief Emit the closing tag for the current highlight.
 */
void html_end_hilite (void)
{
  if (hl_stack[hl_sp] & HL_TT)
    write_string ("</TT>");
  else if (hl_stack[hl_sp] & HL_BF)
    write_string ("</B>");
  else if (hl_stack[hl_sp] & (HL_SL | HL_EM))
    write_string ("</I>");
}


/*!
 * @brief Emit the closing tags of the current environment.
 */
void html_end_env (void)
{
  switch (env_stack[env_sp].env)
    {
    case ENV_DESCRIPTION:
    case ENV_LIST:
      write_break ();
      write_line ("</DL>");
      break;
    case ENV_ENUMERATE:
      write_break ();
      write_line ("</OL>");
      break;
    case ENV_ITEMIZE:
      write_break ();
      write_line ("</UL>");
      break;
    case ENV_TYPEWRITER:
      write_break ();
      write_line ("</PRE></BLOCKQUOTE>");
      break;
    case ENV_INDENT:
      write_break ();
      write_line ("</BLOCKQUOTE>");
      break;
    default:
      abort ();
    }
}


/*!
 * @brief Begin the table of contents.
 */
void html_toc_start (void)
{
}


/*!
 * @brief Emit one table-of-contents entry.
 *
 * @param[in] s  Section number text.
 * @param[in] tp Table-of-contents entry.
 */
void html_toc_line (const uchar *s, const struct toc *tp)
{
  format_string (s, STYLE_NORMAL, FALSE);
  html_begin_link (NULL, tp->ref);
  html_output (tp->title, FALSE);
  html_end_link ();
  write_line ("<BR>");
}


/*!
 * @brief End the table of contents.
 */
void html_toc_end (void)
{
  write_line ("<HR>");
}

/*!
 * @brief Emit the anchor of a level-1 heading.
 *
 * @param[in] ref Section reference number.
 */
void html_heading1 (int ref)
{
  write_break ();
  write_fmt ("<A NAME=\"%d\"></A>", ref);
  write_nl ();
}


/*!
 * @brief Emit a level-2 heading.
 *
 * @param[in] s Heading text. Not NULL.
 */
void html_heading2 (const uchar *s)
{
/*   if (para_flag) */
/*     html_para (); */
  switch (tg_level)
    {
    case 0:
      write_string (s);
      write_line ("<BR>");
      break;
    case 1: case 2: case 3: case 4: case 5: case 6:
      write_fmt ("<H%d>", tg_level);
      html_output (s, FALSE);
      write_fmt ("</H%d>", tg_level);
      write_nl ();
      break;
    default:
      fatal ("Sorry, HTML supports only 6 levels of headings");
    }
  write_nl ();
}


/*!
 * @brief Begin a description list environment.
 */
void html_description (void)
{
  write_break ();
  write_line ("<DL COMPACT>");
  write_nl ();
}


/*!
 * @brief Begin an ordered list environment.
 */
void html_enumerate (void)
{
  write_break ();
  write_line ("<OL>");
  write_nl ();
}


/*!
 * @brief Begin an unordered list environment.
 */
void html_itemize (void)
{
  write_break ();
  write_line ("<UL>");
  write_nl ();
}


/*!
 * @brief Begin an indented block.
 */
void html_indent (void)
{
  write_break ();
  write_line ("<BLOCKQUOTE>");
}


/*!
 * @brief Begin a list environment.
 */
void html_list (void)
{
  write_break ();
  write_line ("<DL COMPACT>");
}


/*!
 * @brief Begin a verbatim block.
 *
 * @param[in] tag_end Tag that terminates the block.
 */
void html_verbatim_start (enum tag tag_end)
{
  write_break ();
  switch (tag_end)
    {
    case TAG_ENDHEADERS:
      html_para ();
      format_string ("Headers:", STYLE_BOLD, FALSE);
      break;
    case TAG_ENDSAMPLECODE:
      html_para ();
      format_string ("Example:", STYLE_BOLD, FALSE);
      write_line ("<BLOCKQUOTE>");
      break;
    case TAG_ENDEXAMPLE:
      write_line ("<BLOCKQUOTE>");
      break;
    default:
      break;
    }
  write_break ();
  write_line ("<PRE>");
}

/*!
 * @brief Emit one verbatim line.
 */
void html_verbatim_line (void)
{
  html_output (input, FALSE);
  write_nl ();
}

/*!
 * @brief End a verbatim block.
 *
 * @param[in] tag_end Tag that terminated the block.
 */
void html_verbatim_end (enum tag tag_end)
{
  write_line ("</PRE>");
  switch (tag_end)
    {
    case TAG_ENDEXAMPLE:
      write_line ("</BLOCKQUOTE>");
      html_para ();
      break;
    case TAG_ENDSAMPLECODE:
      write_line ("</BLOCKQUOTE>");
      break;
    default:
      break;
    }
}


/*!
 * @brief Emit a description-list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void html_description_item (const uchar *s)
{
  write_break ();
  write_string ("<DT>");
  make_elements (s);
  html_elements (STYLE_NORMAL);
  write_string ("<DD>");
}


/*!
 * @brief Emit an ordered-list item.
 */
void html_enumerate_item (void)
{
  write_break ();
  write_line ("<LI>");
}


/*!
 * @brief Emit an unordered-list item.
 */
void html_itemize_item (void)
{
  write_break ();
  write_line ("<LI>");
}


/*!
 * @brief Emit a list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void html_list_item (const uchar *s)
{
  write_break ();
  write_string ("<DT>");
  make_elements (s);
  html_elements (STYLE_NORMAL);
  write_string ("<DD>");
}


/*!
 * @brief Emit a paragraph of normal text.
 */
void html_copy (void)
{
  if (para_flag)
    html_para ();
  switch (env_stack[env_sp].env)
    {
    case ENV_TYPEWRITER:
      html_elements (STYLE_TTY);
      break;
    default:
      html_elements (STYLE_NORMAL);
      break;
    }
}


/*!
 * @brief Emit the HTML document prologue.
 */
void html_start (void)
{
  write_line ("<HTML><HEAD>");
  if (title != NULL)
    {
      write_string ("<TITLE>");
      format_string (title, STYLE_NORMAL, FALSE);
      write_line ("</TITLE>");
    }
  write_line ("</HEAD><BODY>");
}


/*!
 * @brief Emit the HTML document epilogue and the index.
 */
void html_end (void)
{
  if (index_count != 0)
    start_index ();
  if (out)
    {
      write_nl ();
      html_write_index ();
      write_line ("</BODY></HTML>");
    }
}

/*!
 * @brief Begin a prototype block.
 */
void html_prototype_start (void)
{
  if (para_flag)
    html_para ();
  format_string (function_count == 1 ? "Prototype:" : "Prototypes:",
                 STYLE_BOLD, FALSE);
  write_break ();
  html_para ();
}

/*!
 * @brief End a prototype block.
 *
 * @param[in,out] compat Compatibility note buffer.
 */
void html_prototype_end (uchar *compat)
{
  html_elements (STYLE_TTY);
  html_para ();
  if (compat[0] != 0)
    {
      format_string ("Compatibility:", STYLE_BOLD, FALSE);
      html_para ();
      format_string (compat, STYLE_NORMAL, FALSE);
      html_para ();
      compat[0] = 0;
    }
  format_string ("Description:", STYLE_BOLD, FALSE);
}

/*!
 * @brief Begin a "See also" paragraph.
 */
void html_see_also_start (void)
{
  if (para_flag)
    html_para ();
  format_string ("See also: ", STYLE_BOLD, FALSE);
}

/*!
 * @brief Emit one "See also" reference.
 *
 * @param[in] word Reference text. Not NULL.
 * @param[in] s    Remaining list text. Not NULL.
 */
void html_see_also_word (const uchar *word, const uchar *s)
{
  struct word *wp;

  wp = use_reference (word);
  if (wp != NULL)
    {
      if (wp->database == NULL)
        html_begin_link (NULL, wp->ref);
      else
        html_begin_link (wp->database->str, wp->ref);
      html_output (word, FALSE);
      html_end_link ();
    }
  if (*s != 0)
    {
      write_string (",");
      if (output_x >= 60)
        write_nl ();
      else
        write_string (" ");
    }
}

/*!
 * @brief Emit a sample-file reference.
 *
 * @param[in] s Sample file name. Not NULL.
 */
void html_sample_file (const uchar *s)
{
  html_libref_section ("Example");
  format_string (" See ", STYLE_NORMAL, FALSE);
  format_string (s, STYLE_TTY, FALSE);
  para_flag = TRUE;
}

/*!
 * @brief Emit a library-reference section heading.
 *
 * @param[in] s Section title. Not NULL.
 */
void html_libref_section (const uchar *s)
{
  if (para_flag)
    html_para ();
  format_string (s, STYLE_BOLD, FALSE);
  write_string (":");
  para_flag = TRUE;
}

/*!
 * @brief Begin a function documentation block.
 *
 * @param[in] tp Table-of-contents entry for the function.
 */
void html_function_start (const struct toc *tp)
{
  write_line ("<HR>");
  html_heading1 (tp->ref);
  write_string ("<H2>");
  html_output (tp->title, FALSE);
  write_line ("</H2>");
}

/*!
 * @brief Emit one function name within a function block.
 *
 * @param[in] tp Table-of-contents entry for the function.
 * @param[in] s  Function name. Not NULL.
 */
void html_function_function (const struct toc *tp, const uchar *s)
{
  if (index_wp1 != NULL)
    html_index (tp, s, 2);
}

/*!
 * @brief Emit a mini table of contents.
 *
 * @param[in] tp Current table-of-contents entry.
 */
void html_minitoc (const struct toc *tp)
{
  int level;

  if (tp == NULL)
    fatal ("%s:%d: Cannot build minitoc before the first heading",
           input_fname, line_no);
  if (para_flag)
    html_para ();
  level = tp->level;
  tp = tp->next;
  while (tp != NULL && tp->level >= level + 1)
    {
      if (tp->level == level + 1)
        {
          html_begin_link (NULL, tp->ref);
          format_output (tp->title, FALSE);
          html_end_link ();
          write_line ("<BR>");
        }
      tp = tp->next;
    }
}


/*!
 * @brief Register an index entry.
 *
 * @param[in] tp    Table-of-contents entry the entry refers to.
 * @param[in] s     Index entry text. Not NULL.
 * @param[in] level Index level: 0 for main, 1 for i1, 2 for i2.
 */
void html_index (const struct toc *tp, const uchar *s, int level)
{
  struct word *wp;

  switch (level)
    {
    case 0:
      wp = word_add (s);
      assert (tp != NULL);
      if (wp->idx == 0)
        {
          wp->idx = tp->ref;
          if (wp->subidx == NULL)
            ++index_count;
        }
      else if (wp->idx != tp->ref)
        warning (1, "%s:%d: Index entry multiply defined",
                 input_fname, line_no);
      break;
    case 1:
      if (*s == 0)
        index_wp1 = NULL;
      else
        {
          wp = word_add (s);
          index_wp1 = wp;
        }
      break;
    case 2:
      if (index_wp1 == NULL)
        fatal ("%s:%d: %ci2 without %ci1", input_fname, line_no,
               escape, escape);
      if (index_wp1->subidx == NULL)
        {
          index_wp1->subidx = wt_new (37);
          if (index_wp1->idx == 0)
            ++index_count;
        }
      wp = wt_add (index_wp1->subidx, s);
      assert (tp != NULL);
      if (wp->idx == 0)
        wp->idx = tp->ref;
      else if (wp->idx != tp->ref)
        warning (1, "%s:%d: Index entry multiply defined",
                 input_fname, line_no);
      break;
    default:
      abort ();
    }
}


/*!
 * @brief Callback: collect index words that have entries.
 *
 * @param[in] wp Word to examine. Not NULL.
 *
 * @return Always 0.
 */
static int index_add (struct word *wp)
{
  if (wp->idx != 0 || wp->subidx != NULL)
    {
      assert (index_n < index_count);
      index_v[index_n++] = wp;
    }
  return 0;
}

/*!
 * @brief Case-folding map for ASCII.
 */
static const uchar map_ascii[256] =
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f"
  "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f"
  "\x40\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"
  "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x5a\x5b\x5c\x5d\x5e\x5f"
  "\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"
  "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";

/*!
 * @brief Case-folding map for code page 850.
 */
static const uchar map_cp850[256] =
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f"
  "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f"
  "\x40\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"
  "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x5a\x5b\x5c\x5d\x5e\x5f"
  "\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"
  "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\xff"
  "\x63\x75\x65\x61\x61\x61\x61\x63\x65\x65\x65\x69\x69\x69\x61\x61"
  "\x65\x61\x61\x6f\x6f\x6f\x75\x75\x79\x6f\x75\x6f\xff\x6f\xff\xff"
  "\x61\x69\x6f\x75\x6e\x6e\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\x61\x61\x61\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\x61\x61\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\x65\x65\x65\xff\x69\x69\x69\xff\xff\xff\xff\xff\x69\xff"
  "\x6f\x80\x6f\x6f\x6f\x6f\xff\xff\xff\x75\x75\x75\x79\x79\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";

/*!
 * @brief Case-folding map for ISO 8859-1.
 */
static const uchar map_iso8859_1[256] =
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f"
  "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f"
  "\x40\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"
  "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x5a\x5b\x5c\x5d\x5e\x5f"
  "\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"
  "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\x20\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
  "\x61\x61\x61\x61\x61\x61\x61\x63\x65\x65\x65\x65\x69\x69\x69\x69"
  "\xff\x6e\x6f\x6f\x6f\x6f\x6f\xff\x6f\x75\x75\x75\x75\x79\xff\x80"
  "\x61\x61\x61\x61\x61\x61\x61\x63\x65\x65\x65\x65\x69\x69\x69\x69"
  "\xff\x6e\x6f\x6f\x6f\x6f\x6f\xff\x6f\x75\x75\x75\x75\x79\xff\x79";

/*!
 * @brief Compare two strings using a case-folding map.
 *
 * @param[in] s1  First string. Not NULL.
 * @param[in] n1  Length of @p s1.
 * @param[in] s2  Second string. Not NULL.
 * @param[in] n2  Length of @p s2.
 * @param[in] map Case-folding map.
 *
 * @return Negative, zero, or positive, following the qsort convention.
 *
 * @retval -1  @p s1 sorts before @p s2.
 * @retval  0  @p s1 and @p s2 compare equal.
 * @retval  1  @p s1 sorts after @p s2.
 */
static int compare1 (const uchar *s1, size_t n1,
                     const uchar *s2, size_t n2,
                     const uchar *map)
{
  uchar c1, c2;
  int cmp;

  while (n1 != 0 && n2 != 0)
    {
      c1 = map[*s1];
      c2 = map[*s2];
      if (c1 != c2)
        {
          /* Special cases for \ss */
          if (c1 == 0x80)
            {
              cmp = compare1 ("ss", 2, s2, n2 < 2 ? n2 : 2, map_ascii);
              if (cmp != 0)
                return cmp;
              ++s2; --n2;
            }
          else if (c2 == 0x80)
            {
              cmp = compare1 (s1, n1 < 2 ? n1 : 2, "ss", 2, map_ascii);
              if (cmp != 0)
                return cmp;
              ++s1; --n1;
            }
          else if (c1 < c2)
            return -1;
          else
            return 1;
        }
      ++s1; ++s2; --n1; --n2;
    }
  if (n1 != 0)
    return 1;
  if (n2 != 0)
    return -1;
  return 0;
}

/*!
 * @brief qsort callback comparing two index entries.
 *
 * @param[in] p1 Pointer to first entry.
 * @param[in] p2 Pointer to second entry.
 *
 * @return Negative, zero, or positive, following the qsort convention.
 */
static int index_compare (const void *p1, const void *p2)
{
  const struct word *wp1 = *(const struct word **)p1;
  const struct word *wp2 = *(const struct word **)p2;
  int cmp = compare1 (wp1->str, strlen (wp1->str),
                      wp2->str, strlen (wp2->str), index_map);
  if (cmp != 0)
    return cmp;
  return strcmp (wp1->str, wp2->str);
}

/*!
 * @brief Recursively emit an index table.
 *
 * @param[in] wt    Word table to emit. Not NULL.
 * @param[in] level Current nesting level.
 */
static void index_recurse (struct word_table *wt, int level)
{
  int i, j, n;
  struct word **v;

  if (level != 0)
    index_count = wt_count (wt);
  n = index_count;
  index_v = xmalloc (n * sizeof (*index_v));
  index_n = 0;
  wt_walk (wt, index_add);
  assert (index_n == n);
  v = index_v;
  index_count = 0; index_v = NULL;

  qsort (v, n, sizeof (*v), index_compare);
  for (i = 0; i < n; ++i)
    {
      struct word *wp = v[i];
      assert (wp->idx != 0 || wp->subidx != NULL);
      for (j = 0; j < level; ++j)
        write_string ("&nbsp;&nbsp;");
      if (wp->idx != 0)
        html_begin_link (NULL, wp->idx);
      html_output ((const char *)wp->str, FALSE);
      if (wp->idx != 0)
        html_end_link ();
      write_line ("<BR>");
      if (wp->subidx != NULL)
        index_recurse (wp->subidx, level + 1);
    }
  free (v);
}

/*!
 * @brief Emit the collected index to the output file.
 */
static void html_write_index (void)
{
  if (index_count != 0)
    {
      int n = index_count;
      if (output_encoding == ENC_ISO8859_1)
        index_map = map_iso8859_1;
      else
        index_map = map_cp850;
      index_recurse (word_top, 0);
      index_count = n;
    }
}

/*!
 * @brief Emit an HTML fragment anchor.
 *
 * @param[in] s Anchor name. Not NULL.
 */
void html_fragment (const uchar *s)
{
  write_break ();
  write_fmt ("<A NAME=\"%s\"></A>", (const char *)s);
  write_nl ();
}
