/*!
 * @file ipf.h
 * @brief IPF output.
 *
 * Copyright (c) 1993-1998 Eberhard Mattes
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
 * @brief Indentation used for IPF description lists.
 */
#define IPF_DESCRIPTION_INDENT  8

/*!
 * @brief Current IPF highlighting bits.
 */
EXTERN int hl_ipf;
/*!
 * @brief Current IPF highlighting number.
 */
EXTERN int hl_ipf_no;

/*!
 * @brief Open an IPF hyperlink.
 *
 * @param[in] database Database name, or NULL for the current file.
 * @param[in] ref      Reference number.
 */
void ipf_begin_link (const uchar *database, int ref);
/*!
 * @brief Close an IPF hyperlink.
 */
void ipf_end_link (void);
/*!
 * @brief Start a new IPF paragraph.
 */
void ipf_para (void);
/*!
 * @brief Set the IPF margin from an environment stack entry.
 *
 * @param[in] sp Environment stack index.
 */
void ipf_env_margin (int sp);
/*!
 * @brief Render the current element list.
 *
 * @param[in] style Default style to apply.
 */
void ipf_elements (enum style style);
/*!
 * @brief Emit the closing tags of the current environment.
 */
void ipf_end_env (void);
/*!
 * @brief Begin an IPF table of contents.
 */
void ipf_toc_start (void);
/*!
 * @brief Emit one table-of-contents entry.
 *
 * @param[in] s  Section number text.
 * @param[in] tp Table-of-contents entry.
 */
void ipf_toc_line (const uchar *s, const struct toc *tp);
/*!
 * @brief End an IPF table of contents.
 */
void ipf_toc_end (void);
/*!
 * @brief Emit a level-1 heading.
 *
 * @param[in] level  Heading level.
 * @param[in] ref    Section reference number.
 * @param[in] global Non-zero if the section is global.
 * @param[in] flags  Heading flags.
 */
void ipf_heading1 (int level, int ref, int global, unsigned flags);
/*!
 * @brief Emit a level-2 heading.
 *
 * @param[in] s Heading text. Not NULL.
 */
void ipf_heading2 (const uchar *s);
/*!
 * @brief Write a string with the current IPF highlighting.
 *
 * @param[in] p         String. Not NULL.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
void ipf_output (const uchar *p, int may_break);
/*!
 * @brief Set the current IPF left margin.
 *
 * @param[in] margin Margin in characters.
 */
void ipf_margin (int margin);
/*!
 * @brief Change the current IPF highlighting.
 *
 * @param[in] new New highlight bits.
 */
void ipf_hilite (int new);
/*!
 * @brief Begin an IPF description list.
 */
void ipf_description (void);
/*!
 * @brief Begin an IPF ordered list.
 */
void ipf_enumerate (void);
/*!
 * @brief Begin an IPF unordered list.
 */
void ipf_itemize (void);
/*!
 * @brief Begin an IPF verbatim block.
 *
 * @param[in] tag_end Tag that terminates the block.
 */
void ipf_verbatim_start (enum tag tag_end);
/*!
 * @brief Emit one verbatim line.
 */
void ipf_verbatim_line (void);
/*!
 * @brief End an IPF verbatim block.
 *
 * @param[in] tag_end Tag that terminated the block.
 */
void ipf_verbatim_end (enum tag tag_end);
/*!
 * @brief Emit a description-list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void ipf_description_item (const uchar *s);
/*!
 * @brief Emit an ordered-list item.
 */
void ipf_enumerate_item (void);
/*!
 * @brief Emit an unordered-list item.
 */
void ipf_itemize_item (void);
/*!
 * @brief Emit a list item.
 *
 * @param[in] s Item label text. Not NULL.
 */
void ipf_list_item (const uchar *s);
/*!
 * @brief Begin a prototype block.
 */
void ipf_prototype_start (void);
/*!
 * @brief End a prototype block.
 *
 * @param[in,out] compat Compatibility note buffer.
 */
void ipf_prototype_end (uchar *compat);
/*!
 * @brief Emit the IPF document prologue.
 */
void ipf_start (void);
/*!
 * @brief Begin an IPF table.
 *
 * @param[in] do_indent Non-zero to indent the table.
 * @param[in] widths    Column widths. May be NULL if @p wn is 0.
 * @param[in] wn        Number of column widths.
 */
void ipf_table_start (int do_indent, int *widths, int wn);
/*!
 * @brief Emit one IPF table line.
 *
 * @param[in] s  Line text. Not NULL.
 * @param[in] wn Expected number of columns.
 */
void ipf_table_line (const uchar *s, int wn);
/*!
 * @brief End an IPF table.
 *
 * @param[in] do_indent Non-zero if the table was indented.
 */
void ipf_table_end (int do_indent);
/*!
 * @brief Register an IPF index entry.
 *
 * @param[in] s Index entry text. Not NULL.
 */
void ipf_index (const uchar *s);
/*!
 * @brief Begin a "See also" paragraph.
 */
void ipf_see_also_start (void);
/*!
 * @brief Emit one "See also" reference.
 *
 * @param[in] word Reference text. Not NULL.
 * @param[in] s    Remaining list text. Not NULL.
 */
void ipf_see_also_word (const uchar *word, const uchar *s);
/*!
 * @brief Emit a sample-file reference.
 *
 * @param[in] s Sample file name. Not NULL.
 */
void ipf_sample_file (const uchar *s);
/*!
 * @brief Emit a library-reference section heading.
 *
 * @param[in] s Section title. Not NULL.
 */
void ipf_libref_section (const uchar *s);
/*!
 * @brief Begin a function documentation block.
 *
 * @param[in] tp Table-of-contents entry for the function.
 */
void ipf_function_start (const struct toc *tp);
/*!
 * @brief Emit one function name within a function block.
 *
 * @param[in] s Function name. Not NULL.
 */
void ipf_function_function (const uchar *s);
/*!
 * @brief Emit a paragraph of normal text.
 */
void ipf_copy (void);
/*!
 * @brief Emit an IPF mini table of contents.
 *
 * @param[in] tp Current table-of-contents entry.
 */
void ipf_minitoc (const struct toc *tp);
/*!
 * @brief Emit the IPF document epilogue.
 */
void ipf_end (void);
