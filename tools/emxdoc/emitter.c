/*!
 * @file emitter.c
 * @brief Emitter table for emxdoc.
 *
 * Provides the dispatch table used by emxdoc.c to select an output
 * format.  Each row of the table is one emitter, chosen at start-up
 * by emit_find().
 *
 * Copyright (C) 2026 osFree Project.
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
#include "emxdoc.h"
#include "emitter.h"

#include "ipf.h"
#include "html.h"
#include "latex.h"
#include "text.h"
#include "man.h"
#include "ed.h"
#include "md.h"
#include "dokuwiki.h"

/* ------------------------------------------------------------------ */
/*  No-op helpers                                                     */
/* ------------------------------------------------------------------ */

/*!
 * @brief No-op for a method with no parameters.
 */
static void noop_v (void)
{
}

/*!
 * @brief No-op for output().
 * @param[in] p         String. Not NULL.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
static void noop_output (const uchar *p, int may_break)
{
  (void)p;
  (void)may_break;
}

/*!
 * @brief No-op for heading1().
 * @param[in] level  Heading level.
 * @param[in] ref    Reference number.
 * @param[in] global Non-zero if global.
 * @param[in] flags  HF_* flags.
 */
static void noop_heading1 (int level, int ref, int global, unsigned int flags)
{
  (void)level;
  (void)ref;
  (void)global;
  (void)flags;
}

/*!
 * @brief No-op for heading2().
 * @param[in] s Heading text. Not NULL.
 */
static void noop_heading2 (const uchar *s)
{
  (void)s;
}

/*!
 * @brief No-op for description_item(), list_item(), sample_file(),
 *        libref_section(), html_fragment() and see_also_end().
 * @param[in] s String. Not NULL.
 */
static void noop_string (const uchar *s)
{
  (void)s;
}

/*!
 * @brief No-op for verbatim_start().
 * @param[in] tag_end  End tag.
 * @param[in] ptmargin Margin pointer. Not NULL.
 */
static void noop_verbatim_start (enum tag tag_end, int *ptmargin)
{
  (void)tag_end;
  (void)ptmargin;
}

/*!
 * @brief No-op for verbatim_line().
 * @param[in] tag_end End tag.
 * @param[in] tmargin Margin.
 * @param[in] compat  Compatibility string. Not NULL.
 */
static void noop_verbatim_line (enum tag tag_end, int tmargin,
                                uchar *compat)
{
  (void)tag_end;
  (void)tmargin;
  (void)compat;
}

/*!
 * @brief No-op for verbatim_end().
 * @param[in] tag_end End tag.
 */
static void noop_verbatim_end (enum tag tag_end)
{
  (void)tag_end;
}

/*!
 * @brief No-op for prototype_start() and prototype_end().
 * @param[in] compat Compatibility string. Not NULL.
 */
static void noop_prototype (uchar *compat)
{
  (void)compat;
}

/*!
 * @brief No-op for toc_line().
 * @param[in] s  Section number.
 * @param[in] tp TOC entry. Not NULL.
 */
static void noop_toc_line (const uchar *s, const struct toc *tp)
{
  (void)s;
  (void)tp;
}

/*!
 * @brief No-op for minitoc() and function_start().
 * @param[in] tp TOC entry. Not NULL.
 */
static void noop_toc (const struct toc *tp)
{
  (void)tp;
}

/*!
 * @brief No-op for function_function().
 * @param[in] tp TOC entry. Not NULL.
 * @param[in] s  Function name. Not NULL.
 */
static void noop_function_function (const struct toc *tp, const uchar *s)
{
  (void)tp;
  (void)s;
}

/*!
 * @brief No-op for index().
 * @param[in] tp    TOC entry, or NULL.
 * @param[in] s     Index text. Not NULL.
 * @param[in] level 0, 1 or 2.
 */
static void noop_index (const struct toc *tp, const uchar *s, int level)
{
  (void)tp;
  (void)s;
  (void)level;
}

/*!
 * @brief No-op for see_also_word().
 * @param[in] word Reference text. Not NULL.
 * @param[in] s    Remaining text. Not NULL.
 */
static void noop_see_also_word (const uchar *word, const uchar *s)
{
  (void)word;
  (void)s;
}

/*!
 * @brief No-op for table_start().
 * @param[in] do_indent Non-zero to indent.
 * @param[in] widths    Column widths, or NULL.
 * @param[in] wn        Number of columns.
 */
static void noop_table_start (int do_indent, int *widths, int wn)
{
  (void)do_indent;
  (void)widths;
  (void)wn;
}

/*!
 * @brief No-op for table_line().
 * @param[in] s  Row text. Not NULL.
 * @param[in] wn Number of columns.
 */
static void noop_table_line (const uchar *s, int wn)
{
  (void)s;
  (void)wn;
}

/*!
 * @brief No-op for table_end().
 * @param[in] do_indent Non-zero if the table was indented.
 */
static void noop_table_end (int do_indent)
{
  (void)do_indent;
}

/*!
 * @brief No-op for hyphenation().
 * @param[in] name File name. Not NULL.
 */
static void noop_hyphenation (const char *name)
{
  (void)name;
}

/* ------------------------------------------------------------------ */
/*  Emitter table                                                     */
/* ------------------------------------------------------------------ */

/*!
 * @brief All known emitters, terminated by a row with a NULL name.
 *
 * Each row has exactly 47 values: 7 data fields followed by 40
 * function pointers, in the same order as struct emitter.  The
 * description and usage strings are consumed only by usage().
 */
static struct emitter emitters[] = {

  /* ------------------------------------------------------------- */
  /*  IPF                                                          */
  /* ------------------------------------------------------------- */
  {
    "ipf", 'I',
    EMIT_FLAG_NO_OUTPUT_IF_G | EMIT_FLAG_COLOR | EMIT_FLAG_XREF
      | EMIT_FLAG_FIXED_ENCODING,
    250, ENC_CP850,
    "Generate IPF file",
    "[-acfgr] [-n <start>] [-o <output>] [-x <xref>] <input>",
    ipf_start, ipf_end,
    ipf_output,
    noop_v, noop_v,
    ipf_heading1, ipf_heading2,
    ipf_description, ipf_enumerate, ipf_itemize, ipf_indent, ipf_list,
    ipf_end_env,
    ipf_description_item, ipf_enumerate_item, ipf_itemize_item,
    ipf_list_item,
    ipf_copy,
    ipf_verbatim_start, ipf_verbatim_line, ipf_verbatim_end,
    ipf_prototype_start, ipf_prototype_end,
    ipf_toc_start, ipf_toc_line, ipf_toc_end, ipf_minitoc,
    ipf_function_start, ipf_function_function,
    ipf_index,
    ipf_see_also_start, ipf_see_also_word, noop_string,
    ipf_sample_file, ipf_libref_section,
    ipf_table_start, ipf_table_line, ipf_table_end,
    noop_string,
    noop_hyphenation
  },

  /* ------------------------------------------------------------- */
  /*  HTML                                                         */
  /* ------------------------------------------------------------- */
  {
    "html", 'H',
    EMIT_FLAG_LINK_HPT | EMIT_FLAG_XREF | EMIT_FLAG_FIXED_ENCODING,
    4096, ENC_ISO8859_1,
    "Generate HTML file",
    "[-o <output>] [-x <xref>] <input>",
    html_start, html_end,
    html_output,
    html_start_hilite, html_end_hilite,
    html_heading1, html_heading2,
    html_description, html_enumerate, html_itemize, html_indent,
    html_list,
    html_end_env,
    html_description_item, html_enumerate_item, html_itemize_item,
    html_list_item,
    html_copy,
    html_verbatim_start, html_verbatim_line, html_verbatim_end,
    html_prototype_start, html_prototype_end,
    html_toc_start, html_toc_line, html_toc_end, html_minitoc,
    html_function_start, html_function_function,
    html_index,
    html_see_also_start, html_see_also_word, noop_string,
    html_sample_file, html_libref_section,
    noop_table_start, noop_table_line, noop_table_end,
    html_fragment,
    noop_hyphenation
  },

  /* ------------------------------------------------------------- */
  /*  LaTeX                                                        */
  /* ------------------------------------------------------------- */
  {
    "latex", 'L', EMIT_FLAG_SOFT_REFS, 4096, ENC_DEFAULT,
    "Generate LaTeX file",
    "[-fr] [-o <output>] <input>",
    latex_start, latex_end,
    latex_output,
    latex_start_hilite, latex_end_hilite,
    latex_heading1, latex_heading2,
    latex_description, latex_enumerate, latex_itemize, latex_indent,
    latex_list,
    latex_end_env,
    latex_description_item, latex_enumerate_item, latex_itemize_item,
    latex_list_item,
    latex_copy,
    latex_verbatim_start, noop_verbatim_line, latex_verbatim_end,
    latex_prototype_start, latex_prototype_end,
    noop_v, noop_toc_line, noop_v, noop_toc,
    latex_function_start, latex_function_function,
    latex_index,
    latex_see_also_start, latex_see_also_word, noop_string,
    latex_sample_file, latex_libref_section,
    noop_table_start, noop_table_line, noop_table_end,
    noop_string,
    noop_hyphenation
  },

  /* ------------------------------------------------------------- */
  /*  text                                                         */
  /* ------------------------------------------------------------- */
  {
    "text", 'T', EMIT_FLAG_TEXT_STYLE | EMIT_FLAG_SOFT_REFS,
    79, ENC_DEFAULT,
    "Generate text file",
    "[-fr] [-b <number>] [-h <file>] [-o <output>] <input>",
    noop_v, noop_v,
    text_output,
    noop_v, noop_v,
    text_heading1, text_heading2,
    text_description, text_enumerate, text_itemize, text_indent,
    text_list,
    noop_v,
    text_description_item, text_enumerate_item, text_itemize_item,
    text_list_item,
    text_copy,
    text_verbatim_start, text_verbatim_line, noop_verbatim_end,
    text_prototype_start, text_prototype_end,
    noop_v, text_toc_line, noop_v, noop_toc,
    text_function_start, noop_function_function,
    noop_index,
    text_see_also_start, noop_see_also_word, text_see_also_end,
    text_sample_file, text_libref_section,
    text_table_start, text_table_line, noop_table_end,
    noop_string,
    text_hyphenation
  },

  /* ------------------------------------------------------------- */
  /*  man                                                          */
  /* ------------------------------------------------------------- */
  {
    "man", 'N', EMIT_FLAG_COLOR | EMIT_FLAG_XREF, 4096, ENC_UTF_8,
    "Generate man (roff) page",
    "[-cfgr] [-o <output>] [-x <xref>] <input>",
    man_start, man_end,
    man_output,
    man_start_hilite, man_end_hilite,
    man_heading1, man_heading2,
    man_description, man_enumerate, man_itemize, man_indent, man_list,
    man_end_env,
    man_description_item, man_enumerate_item, man_itemize_item,
    man_list_item,
    man_copy,
    man_verbatim_start, man_verbatim_line, man_verbatim_end,
    man_prototype_start, man_prototype_end,
    noop_v, man_toc_line, noop_v, man_minitoc,
    man_function_start, man_function_function,
    man_index,
    man_see_also_start, man_see_also_word, man_see_also_end,
    man_sample_file, man_libref_section,
    man_table_start, man_table_line, man_table_end,
    noop_string,
    noop_hyphenation
  },

  /* ------------------------------------------------------------- */
  /*  emxdoc                                                       */
  /* ------------------------------------------------------------- */
  {
    "ed", 'E', 0, 4096, ENC_DEFAULT,
    "Generate emxdoc (source) file",
    "[-r] [-o <output>] <input>",
    ed_start, ed_end,
    ed_output,
    ed_start_hilite, ed_end_hilite,
    ed_heading1, ed_heading2,
    ed_description, ed_enumerate, ed_itemize, ed_indent, ed_list,
    ed_end_env,
    ed_description_item, ed_enumerate_item, ed_itemize_item,
    ed_list_item,
    ed_copy,
    ed_verbatim_start, ed_verbatim_line, ed_verbatim_end,
    ed_prototype_start, ed_prototype_end,
    ed_toc_start, ed_toc_line, ed_toc_end, ed_minitoc,
    ed_function_start, ed_function_function,
    ed_index,
    ed_see_also_start, ed_see_also_word, ed_see_also_end,
    ed_sample_file, ed_libref_section,
    ed_table_start, ed_table_line, ed_table_end,
    ed_html_fragment,
    noop_hyphenation
  },

  /* ------------------------------------------------------------- */
  /*  Markdown                                                     */
  /* ------------------------------------------------------------- */
  {
    "md", 'G', EMIT_FLAG_COLOR | EMIT_FLAG_XREF, 4096, ENC_UTF_8,
    "Generate Markdown file",
    "[-cfgr] [-o <output>] [-x <xref>] <input>",
    md_start, md_end,
    md_output,
    md_start_hilite, md_end_hilite,
    md_heading1, md_heading2,
    md_description, md_enumerate, md_itemize, md_indent, md_list,
    md_end_env,
    md_description_item, md_enumerate_item, md_itemize_item,
    md_list_item,
    md_copy,
    md_verbatim_start, md_verbatim_line, md_verbatim_end,
    md_prototype_start, md_prototype_end,
    md_toc_start, md_toc_line, md_toc_end, md_minitoc,
    md_function_start, md_function_function,
    md_index,
    md_see_also_start, md_see_also_word, md_see_also_end,
    md_sample_file, md_libref_section,
    md_table_start, md_table_line, md_table_end,
    md_html_fragment,
    noop_hyphenation
  },

  /* ------------------------------------------------------------- */
  /*  DokuWiki                                                     */
  /* ------------------------------------------------------------- */
  {
    "dw", 'W', EMIT_FLAG_COLOR | EMIT_FLAG_XREF, 4096, ENC_UTF_8,
    "Generate DokuWiki file",
    "[-cfgr] [-o <output>] [-x <xref>] <input>",
    dw_start, dw_end,
    dw_output,
    dw_start_hilite, dw_end_hilite,
    dw_heading1, dw_heading2,
    dw_description, dw_enumerate, dw_itemize, dw_indent, dw_list,
    dw_end_env,
    dw_description_item, dw_enumerate_item, dw_itemize_item,
    dw_list_item,
    dw_copy,
    dw_verbatim_start, dw_verbatim_line, dw_verbatim_end,
    dw_prototype_start, dw_prototype_end,
    dw_toc_start, dw_toc_line, dw_toc_end, dw_minitoc,
    dw_function_start, dw_function_function,
    dw_index,
    dw_see_also_start, dw_see_also_word, dw_see_also_end,
    dw_sample_file, dw_libref_section,
    dw_table_start, dw_table_line, dw_table_end,
    dw_html_fragment,
    noop_hyphenation
  },

  /* ------------------------------------------------------------- */
  /*  Terminator                                                   */
  /* ------------------------------------------------------------- */
  {
    NULL, 0, 0, 0, 0, NULL, NULL,
    NULL, NULL, NULL,
    NULL, NULL,
    NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,
    NULL, NULL, NULL,
    NULL,
    NULL,
    NULL, NULL, NULL,
    NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL,
    NULL,
    NULL, NULL, NULL,
    NULL, NULL,
    NULL, NULL, NULL,
    NULL,
    NULL
  }
};

/* ------------------------------------------------------------------ */
/*  Lookup and usage helpers                                          */
/* ------------------------------------------------------------------ */

/*!
 * @brief Look up an emitter by mode character.
 *
 * @param[in] mode Mode character, e.g. 'N'.
 *
 * @return Pointer to the emitter, or NULL if unknown.
 */
struct emitter *emit_find (char mode)
{
  int i;

  for (i = 0; emitters[i].name != NULL; ++i)
    if (emitters[i].mode == mode)
      return &emitters[i];
  return NULL;
}

/*!
 * @brief Write every emitter mode character into @p buf.
 *
 * @param[out] buf  Destination buffer.  Not NULL.
 * @param[in]  size Size of @p buf in bytes.  Must be > 0.
 */
void emit_mode_string (char *buf, size_t size)
{
  int i;
  size_t n = 0;

  if (size == 0)
    return;
  for (i = 0; emitters[i].name != NULL; ++i)
    {
      if (n + 1 >= size)
        fatal ("Emitter mode string overflow");
      buf[n++] = emitters[i].mode;
    }
  buf[n] = 0;
}

/*!
 * @brief Emit the "Usage:" lines for every emitter.
 *
 * @param[in] f Output stream.  Not NULL.
 */
void emit_print_usage (FILE *f)
{
  int i;

  for (i = 0; emitters[i].name != NULL; ++i)
    fprintf (f, "  emxdoc -%c %s\n",
             emitters[i].mode, emitters[i].usage);
}

/*!
 * @brief Emit the "Modes:" lines for every emitter.
 *
 * @param[in] f Output stream.  Not NULL.
 */
void emit_print_modes (FILE *f)
{
  int i;

  for (i = 0; emitters[i].name != NULL; ++i)
    fprintf (f, "  -%c         %s\n",
             emitters[i].mode, emitters[i].summary);
}
