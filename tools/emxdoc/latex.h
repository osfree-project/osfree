/*!
 * @file latex.h
 * @brief LaTeX output.
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


/*!
 * @brief Write a string, escaping LaTeX metacharacters.
 *
 * @param[in] p         String. Not NULL.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
void latex_output (const uchar *p, int may_break);
/*!
 * @brief Emit the opening command for the current highlight.
 */
void latex_start_hilite (void);
/*!
 * @brief Emit the closing command for the current highlight.
 */
void latex_end_hilite (void);
/*!
 * @brief Emit the closing tags of the current environment.
 */
void latex_end_env (void);
/*!
 * @brief Emit the separation between a section number and its title.
 */
void latex_heading1 (void);
/*!
 * @brief Emit a heading.
 *
 * @param[in] s Heading text. Not NULL.
 */
void latex_heading2 (const uchar *s);
/*!
 * @brief Begin a LaTeX description list.
 */
void latex_description (void);
/*!
 * @brief Begin a LaTeX enumerated list.
 */
void latex_enumerate (void);
/*!
 * @brief Begin a LaTeX itemized list.
 */
void latex_itemize (void);
/*!
 * @brief Begin a LaTeX quote block.
 */
void latex_indent (void);
/*!
 * @brief Begin a LaTeX description list.
 */
void latex_list (void);
/*!
 * @brief Begin a LaTeX verbatim block.
 *
 * @param[in] tag_end Tag that terminates the block.
 */
void latex_verbatim_start (enum tag tag_end);
/*!
 * @brief End a LaTeX verbatim block.
 *
 * @param[in] tag_end Tag that terminated the block.
 */
void latex_verbatim_end (enum tag tag_end);
/*!
 * @brief Emit a description-list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void latex_description_item (const uchar *s);
/*!
 * @brief Emit an enumerated-list item.
 */
void latex_enumerate_item (void);
/*!
 * @brief Emit an itemized-list item.
 */
void latex_itemize_item (void);
/*!
 * @brief Emit a list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void latex_list_item (const uchar *s);
/*!
 * @brief Begin a prototype block.
 */
void latex_prototype_start (void);
/*!
 * @brief End a prototype block.
 *
 * @param[in,out] compat Compatibility note buffer.
 */
void latex_prototype_end (uchar *compat);
/*!
 * @brief Register a LaTeX index entry.
 *
 * @param[in] s     Index entry text. Not NULL.
 * @param[in] level Index level: 0 for main, 1 for i1, 2 for i2.
 */
void latex_index (const uchar *s, int level);
/*!
 * @brief Begin a "See also" paragraph.
 */
void latex_see_also_start (void);
/*!
 * @brief Emit one "See also" reference.
 *
 * @param[in] word Reference text. Not NULL.
 * @param[in] s    Remaining list text. Not NULL.
 */
void latex_see_also_word (const uchar *word, const uchar *s);
/*!
 * @brief Emit a sample-file reference.
 *
 * @param[in] s Sample file name. Not NULL.
 */
void latex_sample_file (const uchar *s);
/*!
 * @brief Emit a library-reference section heading.
 *
 * @param[in] s Section title. Not NULL.
 */
void latex_libref_section (const uchar *s);
/*!
 * @brief Begin a function documentation block.
 *
 * @param[in] tp Table-of-contents entry for the function.
 */
void latex_function_start (const struct toc *tp);
/*!
 * @brief Emit one function name within a function block.
 *
 * @param[in] s Function name. Not NULL.
 */
void latex_function_function (const uchar *s);
/*!
 * @brief Emit a paragraph of normal text.
 */
void latex_copy (void);
/*!
 * @brief Emit the LaTeX document prologue.
 */
void latex_start (void);
/*!
 * @brief Emit the LaTeX document epilogue.
 */
void latex_end (void);
