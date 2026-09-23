/*! @file dokuwiki.h
 *  @brief DokuWiki output backend for emxdoc.
 *
 *  Declares the functions emxdoc calls to emit documentation
 *  in DokuWiki format.  Follows the same convention as text.h:
 *  no include guard, no includes, prototypes only.
 *
 *  @copyright Copyright (C) 2026 osFree Project.
 *             License — see LICENSE in the project root.
 */

/*! @brief Initializes DokuWiki output (no-op). */
void dw_start (void);

/*! @brief Finalizes DokuWiki output (no-op). */
void dw_end (void);

/*! @brief Closes the current environment. */
void dw_end_env (void);

/*!
 *  @brief Emits a text fragment with DokuWiki escaping.
 *  @param[in] p         Pointer to a zero-terminated string.
 *  @param[in] may_break Non-zero if a line break is allowed.
 */
void dw_output (const uchar *p, int may_break);

/*! @brief Opens highlighting. */
void dw_start_hilite (void);

/*! @brief Closes highlighting. */
void dw_end_hilite (void);

/*! @brief Starts a section heading. */
void dw_heading1 (void);

/*!
 *  @brief Finishes a section heading.
 *  @param[in] s Heading text.
 */
void dw_heading2 (uchar *s);

/*! @brief Starts the "See also" block. */
void dw_see_also_start (void);

/*!
 *  @brief Finishes the "See also" block.
 *  @param[in] s Ready-made string with the link list.
 */
void dw_see_also_end (const uchar *s);

/*!
 *  @brief Starts a description-list item.
 *  @param[in] s Term.
 */
void dw_description_item (const uchar *s);

/*! @brief Starts an ordered-list item. */
void dw_enumerate_item (void);

/*! @brief Starts an unordered-list item. */
void dw_itemize_item (void);

/*!
 *  @brief Starts a description-list item (list environment).
 *  @param[in] s Term.
 */
void dw_list_item (const uchar *s);

/*! @brief Emits the accumulated elements. */
void dw_copy (void);

/*!
 *  @brief Starts a verbatim/example/samplecode block.
 *  @param[in]  tag_end  End tag of the block.
 *  @param[out] ptmargin Receiver of the current margin.
 */
void dw_verbatim_start (enum tag tag_end, int *ptmargin);

/*!
 *  @brief Emits one verbatim-block line.
 *  @param[in] tag_end End tag of the block.
 *  @param[in] tmargin Current margin.
 *  @param[in] compat  Compatibility string.
 */
void dw_verbatim_line (enum tag tag_end, int tmargin, uchar *compat);

/*!
 *  @brief Finishes a verbatim/example/samplecode block.
 *  @param[in] tag_end End tag of the block.
 */
void dw_verbatim_end (enum tag tag_end);

/*!
 *  @brief Emits the function-section heading.
 *  @param[in] tp TOC node.  Not NULL.
 */
void dw_function (const struct toc *tp);

/*!
 *  @brief Starts a prototype block.
 *  @param[in] compat Compatibility string.
 */
void dw_prototype_start (uchar *compat);

/*! @brief Finishes a prototype block. */
void dw_prototype_end (void);

/*!
 *  @brief Emits one TOC line.
 *  @param[in] s  Section number.
 *  @param[in] tp TOC node.  Not NULL.
 */
void dw_toc_line (const uchar *s, const struct toc *tp);

/*!
 *  @brief Emits the reference to a sample file.
 *  @param[in] s File name.
 */
void dw_sample_file (const uchar *s);

/*!
 *  @brief Emits a libref section heading.
 *  @param[in] s Section title.
 */
void dw_libref_section (const uchar *s);

/*! @brief Starts the description environment. */
void dw_description (void);

/*! @brief Starts the enumerate environment. */
void dw_enumerate (void);

/*! @brief Starts the itemize environment. */
void dw_itemize (void);

/*! @brief Starts the indent environment. */
void dw_indent (void);

/*! @brief Starts the list environment. */
void dw_list (void);
