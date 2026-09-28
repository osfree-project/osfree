/*!
 * @file md.h
 * @brief Markdown output emitter for emxdoc.
 *
 * Declares the functions emxdoc calls to emit documentation in
 * Markdown format.  Follows the same convention as text.h:
 * no include guard, no includes, prototypes only.
 *
 * @copyright Copyright (C) 2026 osFree Project.
 *            License - see LICENSE in the project root.
 */

/*! @brief Initializes Markdown output (no-op). */
void md_start (void);

/*! @brief Finalizes Markdown output (no-op). */
void md_end (void);

/*! @brief Closes the current environment. */
void md_end_env (void);

/*!
 *  @brief Emits a text fragment with Markdown escaping.
 *  @param[in] p         Pointer to a zero-terminated string.
 *  @param[in] may_break Non-zero if a line break is allowed.
 */
void md_output (const uchar *p, int may_break);

/*! @brief Opens highlighting. */
void md_start_hilite (void);

/*! @brief Closes highlighting. */
void md_end_hilite (void);

/*!
 *  @brief Starts a section heading.
 *  @param[in] level  Heading level (unused).
 *  @param[in] ref    Reference number (unused).
 *  @param[in] global Non-zero if the section is global (unused).
 *  @param[in] flags  Heading flags (unused).
 */
void md_heading1 (int level, int ref, int global, unsigned flags);

/*!
 *  @brief Finishes a section heading.
 *  @param[in] s Heading text. Not NULL.
 */
void md_heading2 (const uchar *s);

/*! @brief Starts the "See also" block. */
void md_see_also_start (void);

/*!
 *  @brief Emits one "See also" reference (no-op; see also words are
 *         collected in the accumulated list passed to md_see_also_end).
 *  @param[in] word Reference text. Not NULL.
 *  @param[in] s    Remaining list text. Not NULL.
 */
void md_see_also_word (const uchar *word, const uchar *s);

/*!
 *  @brief Finishes the "See also" block.
 *  @param[in] s Ready-made string with the link list. Not NULL.
 */
void md_see_also_end (const uchar *s);

/*!
 *  @brief Starts a description-list item.
 *  @param[in] s Term. Not NULL.
 */
void md_description_item (const uchar *s);

/*! @brief Starts an ordered-list item. */
void md_enumerate_item (void);

/*! @brief Starts an unordered-list item. */
void md_itemize_item (void);

/*!
 *  @brief Starts a description-list item (list environment).
 *  @param[in] s Term. Not NULL.
 */
void md_list_item (const uchar *s);

/*! @brief Emits the accumulated elements. */
void md_copy (void);

/*!
 *  @brief Starts a verbatim/example/samplecode block.
 *  @param[in]  tag_end  End tag of the block.
 *  @param[out] ptmargin Receiver of the current margin. Not NULL.
 */
void md_verbatim_start (enum tag tag_end, int *ptmargin);

/*!
 *  @brief Emits one verbatim-block line.
 *  @param[in] tag_end End tag of the block.
 *  @param[in] tmargin Current margin.
 *  @param[in] compat  Compatibility string.
 */
void md_verbatim_line (enum tag tag_end, int tmargin, uchar *compat);

/*!
 *  @brief Finishes a verbatim/example/samplecode block.
 *  @param[in] tag_end End tag of the block.
 */
void md_verbatim_end (enum tag tag_end);

/*!
 *  @brief Emits the function-section heading.
 *  @param[in] tp TOC node. Not NULL.
 */
void md_function_start (const struct toc *tp);

/*!
 *  @brief Emits one function name within a function block (no-op).
 *  @param[in] tp TOC node. Not NULL.
 *  @param[in] s  Function name. Not NULL.
 */
void md_function_function (const struct toc *tp, const uchar *s);

/*!
 *  @brief Starts a prototype block.
 *  @param[in] compat Compatibility string (unused).
 */
void md_prototype_start (uchar *compat);

/*!
 *  @brief Finishes a prototype block.
 *  @param[in] compat Compatibility string (unused; md handles compat
 *                    inline in md_prototype_start).
 */
void md_prototype_end (uchar *compat);

/*!
 *  @brief Emits one TOC line.
 *  @param[in] s  Section number.
 *  @param[in] tp TOC node. Not NULL.
 */
void md_toc_line (const uchar *s, const struct toc *tp);

/*! @brief Begin the table of contents (no-op). */
void md_toc_start (void);

/*! @brief End the table of contents (no-op). */
void md_toc_end (void);

/*!
 *  @brief Emits a mini table of contents (no-op).
 *  @param[in] tp Current table-of-contents entry. Not NULL.
 */
void md_minitoc (const struct toc *tp);

/*!
 *  @brief Register an index entry (no-op; Markdown has no index).
 *  @param[in] tp    TOC entry, or NULL.
 *  @param[in] s     Index entry text. Not NULL.
 *  @param[in] level 0, 1 or 2.
 */
void md_index (const struct toc *tp, const uchar *s, int level);

/*!
 *  @brief Emits the reference to a sample file.
 *  @param[in] s File name. Not NULL.
 */
void md_sample_file (const uchar *s);

/*!
 *  @brief Emits a libref section heading.
 *  @param[in] s Section title. Not NULL.
 */
void md_libref_section (const uchar *s);

/*! @brief Starts the description environment. */
void md_description (void);

/*! @brief Starts the enumerate environment. */
void md_enumerate (void);

/*! @brief Starts the itemize environment. */
void md_itemize (void);

/*! @brief Starts the indent environment. */
void md_indent (void);

/*! @brief Starts the list environment. */
void md_list (void);

/*! @brief Begin a table block (no-op; Markdown has no tables). */
void md_table_start (int do_indent, int *widths, int wn);

/*!
 *  @brief Emit one table row (no-op).
 *  @param[in] s  Row text. Not NULL.
 *  @param[in] wn Number of columns.
 */
void md_table_line (const uchar *s, int wn);

/*!
 *  @brief End a table block (no-op).
 *  @param[in] do_indent Non-zero if the table was indented.
 */
void md_table_end (int do_indent);

/*!
 *  @brief Emit an HTML fragment anchor (no-op for Markdown).
 *  @param[in] s Anchor name. Not NULL.
 */
void md_html_fragment (const uchar *s);
