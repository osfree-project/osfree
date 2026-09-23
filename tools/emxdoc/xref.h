/*!
 * @file xref.h
 * @brief Manage cross references.
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
 * @brief Write one keyword line to the output file.
 *
 * @param[in] p Keyword text. Not NULL.
 */
void write_keyword (const uchar *p);
/*!
 * @brief Define a document label.
 *
 * @param[in] p   Label name. Not NULL.
 * @param[in] ref Reference number.
 * @param[in] msg Kind of definition, used in diagnostics. Not NULL.
 *
 * @return Pointer to the word representing the label.
 */
struct word *define_label (const uchar *p, int ref, const char *msg);
/*!
 * @brief Resolve a reference to a document label.
 *
 * @param[in] p Label name. Not NULL.
 *
 * @return Pointer to the referenced word, or NULL if unresolved.
 */
struct word *use_reference (const uchar *p);
/*!
 * @brief Read or write global cross-reference information for one file.
 *
 * @param[in] name File name. Not NULL.
 */
void make_global (const uchar *name);
/*!
 * @brief Write one or more keyword lines.
 *
 * @param[in] s Whitespace separated keyword list. Not NULL.
 */
void keywords_keyword (const uchar *s);
/*!
 * @brief Emit the prologue of a keyword file.
 *
 * @param[in] fname Input file name. Not NULL.
 */
void keywords_start (const char *fname);
/*!
 * @brief Read a cross-reference file.
 *
 * @param[in] fname Cross-reference file name. Not NULL.
 * @param[in] th    Head of the table of contents. Not NULL.
 */
void read_xref (const char *fname, struct toc *th);
