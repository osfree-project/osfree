/*!
 * @file text.h
 * @brief Text output.
 *
 * Copyright (c) 1993-1995 Eberhard Mattes
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
 * @brief Write a string with the current formatting.
 *
 * @param[in] p         String. Not NULL.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
void text_output (const uchar *p, int may_break);
/*!
 * @brief Emit the start of a level-1 heading.
 */
void text_heading1 (void);
/*!
 * @brief Emit a heading.
 *
 * @param[in,out] s Heading text; overwritten with the underline.
 */
void text_heading2 (uchar *s);
/*!
 * @brief Begin a "See also" paragraph.
 */
void text_see_also_start (void);
/*!
 * @brief Emit the "See also" list.
 *
 * @param[in] s Comma separated reference list. Not NULL.
 */
void text_see_also_end (const uchar *s);
/*!
 * @brief Emit a description-list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void text_description_item (const uchar *s);
/*!
 * @brief Emit an enumerated-list item.
 */
void text_enumerate_item (void);
/*!
 * @brief Emit an itemized-list item.
 */
void text_itemize_item (void);
/*!
 * @brief Emit a list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void text_list_item (const uchar *s);
/*!
 * @brief Emit a paragraph of normal text.
 */
void text_copy (void);
/*!
 * @brief Begin a text verbatim block.
 *
 * @param[in]     tag_end  Tag that terminates the block.
 * @param[in,out] ptmargin Top-margin pointer to adjust. Not NULL.
 */
void text_verbatim_start (enum tag tag_end, int *ptmargin);
/*!
 * @brief Emit one verbatim line.
 *
 * @param[in]     tag_end Tag that terminates the block.
 * @param[in]     tmargin Top margin.
 * @param[in,out] compat  Compatibility buffer. Not NULL.
 */
void text_verbatim_line (enum tag tag_end, int tmargin, uchar *compat);
/*!
 * @brief Emit a function documentation separator.
 */
void text_function (void);
/*!
 * @brief Begin a prototype block.
 *
 * @param[in,out] compat Compatibility buffer. Not NULL.
 */
void text_prototype_start (uchar *compat);
/*!
 * @brief End a prototype block.
 */
void text_prototype_end (void);
/*!
 * @brief Emit one table-of-contents entry.
 *
 * @param[in] s  Section number text.
 * @param[in] tp Table-of-contents entry.
 */
void text_toc_line (const uchar *s, const struct toc *tp);
/*!
 * @brief Begin a text table.
 *
 * @param[in]     do_indent Non-zero to indent the table.
 * @param[in,out] ptmargin  Top-margin pointer to adjust. Not NULL.
 */
void text_table_start (int do_indent, int *ptmargin);
/*!
 * @brief Emit one table line.
 *
 * @param[in] s       Line text. Not NULL.
 * @param[in] tmargin Top margin.
 */
void text_table_line (const uchar *s, int tmargin);
/*!
 * @brief Emit a sample-file reference.
 *
 * @param[in] s Sample file name. Not NULL.
 */
void text_sample_file (const uchar *s);
/*!
 * @brief Emit a library-reference section heading.
 *
 * @param[in] s Section title. Not NULL.
 */
void text_libref_section (const uchar *s);
/*!
 * @brief Load a hyphenation table.
 *
 * @param[in] name File name. Not NULL.
 */
void text_hyphenation (const char *name);
