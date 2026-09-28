/*!
 * @file emitter.h
 * @brief Unified emitter interface for emxdoc.
 *
 * Every output format implements the same set of function
 * pointers.  emxdoc.c dispatches through the single global
 * pointer @c bd; no @c switch (mode) remains in the core.
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

/*!
 * @brief Emitter flags controlling mode-specific behaviour.
 */
#define EMIT_FLAG_TEXT_STYLE   0x0001
#define EMIT_FLAG_LINK_HPT     0x0002
#define EMIT_FLAG_SOFT_REFS    0x0004

/*!
 * @brief One output format.
 *
 * All pointers must be non-NULL; no-op helpers are provided in
 * emitter.c for unused entries.
 *
 * The signatures are the union of what the individual emitters
 * actually need.  An emitter that does not use a parameter must
 * ignore it with @c (void)param;.
 */
struct emitter
{
  const char *name;             /*!< Format name for diagnostics. */
  char mode;                    /*!< Command-line mode character. */
  unsigned int flags;           /*!< EMIT_FLAG_* bits. */

  /*! @brief Emit document prologue. */
  void (*start)   (void);
  /*! @brief Emit document epilogue. */
  void (*end)     (void);

  /*!
   * @brief Emit a text fragment.
   * @param[in] p         String.  Not NULL.
   * @param[in] may_break Non-zero if a line break may be inserted.
   */
  void (*output)  (const uchar *p, int may_break);

  /*! @brief Open highlighting according to hl_stack. */
  void (*start_hilite) (void);
  /*! @brief Close highlighting according to hl_stack. */
  void (*end_hilite)   (void);

  /*!
   * @brief Emit a section heading prologue.
   * @param[in] level  Heading level (0 for %h= / %h-).
   * @param[in] ref    Reference number.
   * @param[in] global Non-zero if the section is global.
   * @param[in] flags  HF_* flags.
   */
  void (*heading1) (int level, int ref, int global, unsigned int flags);

  /*!
   * @brief Emit a section heading body.
   * @param[in] s Heading text (with section number prefix).  Not NULL.
   */
  void (*heading2) (const uchar *s);

  /*! @brief Begin a description environment. */
  void (*description) (void);
  /*! @brief Begin an enumerate environment. */
  void (*enumerate)   (void);
  /*! @brief Begin an itemize environment. */
  void (*itemize)     (void);
  /*! @brief Begin an indent environment. */
  void (*indent)      (void);
  /*! @brief Begin a list environment. */
  void (*list)        (void);

  /*! @brief Close the current environment. */
  void (*end_env) (void);

  /*!
   * @brief Emit a description-list item.
   * @param[in] s Item term.  Not NULL.
   */
  void (*description_item) (const uchar *s);
  /*! @brief Emit an enumerate-list item. */
  void (*enumerate_item)   (void);
  /*! @brief Emit an itemize-list item. */
  void (*itemize_item)     (void);
  /*!
   * @brief Emit a list item.
   * @param[in] s Item term.  Not NULL.
   */
  void (*list_item)        (const uchar *s);

  /*! @brief Emit the current paragraph (elements[]). */
  void (*copy) (void);

  /*!
   * @brief Begin a verbatim/example/samplecode/headers block.
   * @param[in]  tag_end  End tag of the block.
   * @param[out] ptmargin Receiver of the current margin, or NULL.
   */
  void (*verbatim_start) (enum tag tag_end, int *ptmargin);

  /*!
   * @brief Emit one verbatim-block line.
   * @param[in] tag_end End tag of the block.
   * @param[in] tmargin Current margin.
   * @param[in] compat  Compatibility string (may be empty).
   */
  void (*verbatim_line) (enum tag tag_end, int tmargin, uchar *compat);

  /*!
   * @brief End a verbatim/example/samplecode/headers block.
   * @param[in] tag_end End tag of the block.
   */
  void (*verbatim_end) (enum tag tag_end);

  /*!
   * @brief Begin a prototype block.
   * @param[in] compat Compatibility string (may be empty).  Not NULL.
   */
  void (*prototype_start) (uchar *compat);

  /*!
   * @brief End a prototype block.
   * @param[in] compat Compatibility string (may be empty).  Not NULL.
   */
  void (*prototype_end) (uchar *compat);

  /*! @brief Begin the table of contents. */
  void (*toc_start) (void);
  /*!
   * @brief Emit one TOC entry.
   * @param[in] s  Section number with alignment padding.
   * @param[in] tp TOC entry.  Not NULL.
   */
  void (*toc_line) (const uchar *s, const struct toc *tp);
  /*! @brief End the table of contents. */
  void (*toc_end)   (void);

  /*!
   * @brief Emit a mini table of contents.
   * @param[in] tp Current TOC entry.  Not NULL.
   */
  void (*minitoc)   (const struct toc *tp);

  /*!
   * @brief Begin a function documentation block.
   * @param[in] tp TOC entry for the function.  Not NULL.
   */
  void (*function_start) (const struct toc *tp);

  /*!
   * @brief Emit one function name within a function block.
   * @param[in] tp TOC entry for the enclosing function.  Not NULL.
   * @param[in] s  Function name.  Not NULL.
   */
  void (*function_function) (const struct toc *tp, const uchar *s);

  /*!
   * @brief Register an index entry.
   * @param[in] tp    TOC entry the entry refers to, or NULL.
   * @param[in] s     Index entry text.  Not NULL.
   * @param[in] level 0 for %index, 1 for %i1, 2 for %i2.
   */
  void (*index) (const struct toc *tp, const uchar *s, int level);

  /*! @brief Begin a "See also" block. */
  void (*see_also_start) (void);

  /*!
   * @brief Emit one "See also" reference.
   * @param[in] word Reference text.  Not NULL.
   * @param[in] s    Remaining list text.  Not NULL.
   */
  void (*see_also_word) (const uchar *word, const uchar *s);

  /*!
   * @brief End a "See also" block.
   * @param[in] s Accumulated reference list.  Not NULL.
   */
  void (*see_also_end) (const uchar *s);

  /*!
   * @brief Emit a reference to a sample file.
   * @param[in] s File name.  Not NULL.
   */
  void (*sample_file) (const uchar *s);

  /*!
   * @brief Emit a library-reference section heading.
   * @param[in] s Section title.  Not NULL.
   */
  void (*libref_section) (const uchar *s);

  /*!
   * @brief Begin a table block.
   * @param[in] do_indent Non-zero to indent the table.
   * @param[in] widths    Column widths (may be NULL).
   * @param[in] wn        Number of columns.
   */
  void (*table_start) (int do_indent, int *widths, int wn);

  /*!
   * @brief Emit one table row.
   * @param[in] s  Row text with 0xb3 cell separators.  Not NULL.
   * @param[in] wn Expected number of columns.
   */
  void (*table_line) (const uchar *s, int wn);

  /*!
   * @brief End a table block.
   * @param[in] do_indent Non-zero if the table was indented.
   */
  void (*table_end) (int do_indent);

  /*!
   * @brief Emit an HTML fragment anchor.
   * @param[in] s Anchor name.  Not NULL.
   */
  void (*html_fragment) (const uchar *s);

  /*!
   * @brief Load a hyphenation table.
   * @param[in] name File name.  Not NULL.
   */
  void (*hyphenation) (const char *name);
};

/*! @brief The emitter selected at start-up. */
EXTERN struct emitter *bd INIT (NULL);

/*!
 * @brief Look up an emitter by mode character.
 *
 * @param[in] mode Mode character, e.g. 'N'.
 *
 * @return Pointer to the emitter, or NULL if unknown.
 */
struct emitter *emit_find (char mode);
