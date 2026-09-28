/*!
 * @file man.h
 * @brief man (roff) output emitter for emxdoc.
 *
 * Declares the functions emxdoc calls to emit documentation
 * in man format.  Follows the same convention as text.h:
 * no include guard, no includes, prototypes only.
 *
 * @copyright Copyright (C) 2026 osFree Project.
 *            License - see LICENSE in the project root.
 */

/*! @brief Initializes man-page output and prints .TH and .SH NAME. */
void man_start (void);

/*! @brief Finalizes man-page output, emitting the index. */
void man_end (void);

/*! @brief Closes the current environment (list, indent, etc.). */
void man_end_env (void);

/*!
 * @brief Emits a text fragment with roff escaping.
 * @param[in] p         Pointer to a zero-terminated string.
 * @param[in] may_break Non-zero if a line break is allowed.
 */
void man_output (const uchar *p, int may_break);

/*! @brief Opens highlighting according to hl_stack. */
void man_start_hilite (void);

/*! @brief Closes highlighting according to hl_stack. */
void man_end_hilite (void);

/*!
 * @brief Starts a section heading (.SH or .SS).
 * @param[in] level  Heading level (0 for %h= / %h-).
 * @param[in] ref    Reference number.
 * @param[in] global Non-zero if the section is global.
 * @param[in] flags  HF_* flags.
 */
void man_heading1 (int level, int ref, int global, unsigned flags);

/*!
 * @brief Finishes a section heading.
 * @param[in] s Heading text with section number prefix. Not NULL.
 */
void man_heading2 (const uchar *s);

/*! @brief Starts the "See also" block. */
void man_see_also_start (void);

/*!
 * @brief Emits one "See also" reference.
 * @param[in] word Reference text. Not NULL.
 * @param[in] s    Remaining list text. Not NULL.
 */
void man_see_also_word (const uchar *word, const uchar *s);

/*!
 * @brief Finishes the "See also" block.
 * @param[in] s Ready-made string with the link list. Not NULL.
 */
void man_see_also_end (const uchar *s);

/*!
 * @brief Starts a description-list item.
 * @param[in] s Term. Not NULL.
 */
void man_description_item (const uchar *s);

/*! @brief Starts an ordered-list item. */
void man_enumerate_item (void);

/*! @brief Starts an unordered-list item. */
void man_itemize_item (void);

/*!
 * @brief Starts a description-list item (list environment).
 * @param[in] s Term. Not NULL.
 */
void man_list_item (const uchar *s);

/*! @brief Emits the accumulated elements. */
void man_copy (void);

/*!
 * @brief Starts a verbatim/example/samplecode/headers block.
 * @param[in]  tag_end  End tag of the block.
 * @param[out] ptmargin Receiver of the current margin. Not NULL.
 */
void man_verbatim_start (enum tag tag_end, int *ptmargin);

/*!
 * @brief Emits one verbatim-block line.
 * @param[in] tag_end End tag of the block.
 * @param[in] tmargin Current margin.
 * @param[in] compat  Compatibility string (unused).
 */
void man_verbatim_line (enum tag tag_end, int tmargin, uchar *compat);

/*!
 * @brief Finishes a verbatim/example/samplecode block.
 * @param[in] tag_end End tag of the block.
 */
void man_verbatim_end (enum tag tag_end);

/*!
 * @brief Emits the function-section heading.
 * @param[in] tp TOC node. Not NULL.
 */
void man_function_start (const struct toc *tp);

/*!
 * @brief Emits one function name within a function block.
 *
 * Registers the function name in the sub-index of the current %i1.
 *
 * @param[in] tp TOC node of the enclosing function. Not NULL.
 * @param[in] s  Function name. Not NULL.
 */
void man_function_function (const struct toc *tp, const uchar *s);

/*!
 * @brief Starts a prototype block.
 * @param[in] compat Compatibility string (may be empty). Not NULL.
 */
void man_prototype_start (uchar *compat);

/*!
 * @brief Finishes a prototype block and emits COMPATIBILITY.
 * @param[in] compat Compatibility string (may be empty). Not NULL.
 */
void man_prototype_end (uchar *compat);

/*!
 * @brief Emits one TOC line.
 * @param[in] s  Section number with alignment padding.
 * @param[in] tp TOC node. Not NULL.
 */
void man_toc_line (const uchar *s, const struct toc *tp);

/*!
 * @brief Emits the reference to a sample file.
 * @param[in] s File name. Not NULL.
 */
void man_sample_file (const uchar *s);

/*!
 * @brief Emits a libref section heading.
 * @param[in] s Section title. Not NULL.
 */
void man_libref_section (const uchar *s);

/*! @brief Starts the description environment. */
void man_description (void);

/*! @brief Starts the enumerate environment. */
void man_enumerate (void);

/*! @brief Starts the itemize environment. */
void man_itemize (void);

/*! @brief Starts the indent environment. */
void man_indent (void);

/*! @brief Starts the list environment. */
void man_list (void);

/*!
 * @brief Registers an index entry.
 * @param[in] tp    TOC entry the entry refers to, or NULL.
 * @param[in] s     Index entry text. Not NULL.
 * @param[in] level 0 for %index, 1 for %i1, 2 for %i2.
 */
void man_index (const struct toc *tp, const uchar *s, int level);

/*!
 * @brief Emits a mini table of contents.
 * @param[in] tp Current table-of-contents entry. Not NULL.
 */
void man_minitoc (const struct toc *tp);

/*!
 * @brief Begins a table block (.TS).
 * @param[in] do_indent Non-zero to indent the table.
 * @param[in] widths    Column widths.
 * @param[in] wn        Number of columns.
 */
void man_table_start (int do_indent, int *widths, int wn);

/*!
 * @brief Emits one table row.
 * @param[in] s  Row text with 0xb3 cell separators. Not NULL.
 * @param[in] wn Expected number of columns.
 */
void man_table_line (const uchar *s, int wn);

/*!
 * @brief Ends a table block (.TE).
 * @param[in] do_indent Non-zero if the table was indented.
 */
void man_table_end (int do_indent);
