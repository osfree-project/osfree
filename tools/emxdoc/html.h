/*!
 * @file html.h
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


/*!
 * @brief Write a string, escaping HTML metacharacters.
 *
 * @param[in] p         String. Not NULL.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
void html_output (const uchar *p, int may_break);
/*!
 * @brief Render the current element list.
 *
 * @param[in] style Default style to apply.
 */
void html_elements (enum style style);
/*!
 * @brief Emit the opening tag for the current highlight.
 */
void html_start_hilite (void);
/*!
 * @brief Emit the closing tag for the current highlight.
 */
void html_end_hilite (void);
/*!
 * @brief Emit the closing tags of the current environment.
 */
void html_end_env (void);
/*!
 * @brief Begin the table of contents.
 */
void html_toc_start (void);
/*!
 * @brief Emit one table-of-contents entry.
 *
 * @param[in] s  Section number text.
 * @param[in] tp Table-of-contents entry.
 */
void html_toc_line (const uchar *s, const struct toc *tp);
/*!
 * @brief End the table of contents.
 */
void html_toc_end (void);
/*!
 * @brief Emit the anchor of a level-1 heading.
 *
 * @param[in] ref Section reference number.
 */
void html_heading1 (int ref);
/*!
 * @brief Emit a level-2 heading.
 *
 * @param[in] s Heading text. Not NULL.
 */
void html_heading2 (const uchar *s);
/*!
 * @brief Begin a description list environment.
 */
void html_description (void);
/*!
 * @brief Begin an ordered list environment.
 */
void html_enumerate (void);
/*!
 * @brief Begin an unordered list environment.
 */
void html_itemize (void);
/*!
 * @brief Begin an indented block.
 */
void html_indent (void);
/*!
 * @brief Begin a list environment.
 */
void html_list (void);
/*!
 * @brief Begin a verbatim block.
 *
 * @param[in] tag_end Tag that terminates the block.
 */
void html_verbatim_start (enum tag tag_end);
/*!
 * @brief Emit one verbatim line.
 */
void html_verbatim_line (void);
/*!
 * @brief End a verbatim block.
 *
 * @param[in] tag_end Tag that terminated the block.
 */
void html_verbatim_end (enum tag tag_end);
/*!
 * @brief Emit a description-list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void html_description_item (const uchar *s);
/*!
 * @brief Emit an ordered-list item.
 */
void html_enumerate_item (void);
/*!
 * @brief Emit an unordered-list item.
 */
void html_itemize_item (void);
/*!
 * @brief Emit a list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void html_list_item (const uchar *s);
/*!
 * @brief Emit a paragraph of normal text.
 */
void html_copy (void);
/*!
 * @brief Emit the HTML document prologue.
 */
void html_start (void);
/*!
 * @brief Emit the HTML document epilogue and the index.
 */
void html_end (void);
/*!
 * @brief Emit a mini table of contents.
 *
 * @param[in] tp Current table-of-contents entry.
 */
void html_minitoc (const struct toc *tp);
/*!
 * @brief Begin a prototype block.
 */
void html_prototype_start (void);
/*!
 * @brief End a prototype block.
 *
 * @param[in,out] compat Compatibility note buffer.
 */
void html_prototype_end (uchar *compat);
/*!
 * @brief Register an index entry.
 *
 * @param[in] tp    Table-of-contents entry the entry refers to.
 * @param[in] s     Index entry text. Not NULL.
 * @param[in] level Index level: 0 for main, 1 for i1, 2 for i2.
 */
void html_index (const struct toc *tp, const uchar *s, int level);
/*!
 * @brief Begin a "See also" paragraph.
 */
void html_see_also_start (void);
/*!
 * @brief Emit one "See also" reference.
 *
 * @param[in] word Reference text. Not NULL.
 * @param[in] s    Remaining list text. Not NULL.
 */
void html_see_also_word (const uchar *word, const uchar *s);
/*!
 * @brief Emit a sample-file reference.
 *
 * @param[in] s Sample file name. Not NULL.
 */
void html_sample_file (const uchar *s);
/*!
 * @brief Emit a library-reference section heading.
 *
 * @param[in] s Section title. Not NULL.
 */
void html_libref_section (const uchar *s);
/*!
 * @brief Begin a function documentation block.
 *
 * @param[in] tp Table-of-contents entry for the function.
 */
void html_function_start (const struct toc *tp);
/*!
 * @brief Emit one function name within a function block.
 *
 * @param[in] tp Table-of-contents entry for the function.
 * @param[in] s  Function name. Not NULL.
 */
void html_function_function (const struct toc *tp, const uchar *s);
/*!
 * @brief Emit an HTML fragment anchor.
 *
 * @param[in] s Anchor name. Not NULL.
 */
void html_fragment (const uchar *s);
