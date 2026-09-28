/*!
 * @file ed.h
 * @brief emxdoc (source) output emitter for emxdoc.
 *
 * Reconstructs canonical emxdoc source from the parsed
 * representation.  Works entirely through the public emxdoc.h
 * interface; does not touch any private state of emxdoc.c.
 *
 * @copyright Copyright (C) 2026 osFree Project.
 *            License - see LICENSE in the project root.
 */

/*! @brief Initializes emxdoc-source output. */
void ed_start (void);
/*! @brief Finalizes emxdoc-source output. */
void ed_end (void);

/*!
 * @brief Emits text verbatim.
 * @param[in] p         String.  Not NULL.
 * @param[in] may_break Unused.
 */
void ed_output (const uchar *p, int may_break);

/*! @brief Opens an inline style according to hl_stack. */
void ed_start_hilite (void);
/*! @brief Closes an inline style according to hl_stack. */
void ed_end_hilite (void);

/*!
 * @brief Emits a canonical %hN / %h= / %h- prologue.
 * @param[in] level  Heading level (0 for %h= / %h-).
 * @param[in] ref    Reference number (unused).
 * @param[in] global Non-zero if the section is global (unused).
 * @param[in] flags  HF_* flags (unused).
 */
void ed_heading1 (int level, int ref, int global, unsigned int flags);

/*!
 * @brief Emits a canonical %hN / %h= / %h- body.
 * @param[in] s Heading text with section number prefix.  Not NULL.
 */
void ed_heading2 (const uchar *s);

/*! @brief Emits %description. */
void ed_description (void);
/*! @brief Emits %enumerate. */
void ed_enumerate (void);
/*! @brief Emits %itemize. */
void ed_itemize (void);
/*! @brief Emits %indent. */
void ed_indent (void);
/*! @brief Emits %list. */
void ed_list (void);
/*! @brief Emits the closing tag of the current environment. */
void ed_end_env (void);

/*!
 * @brief Emits a %item tag with a term.
 * @param[in] s Term.  Not NULL.
 */
void ed_description_item (const uchar *s);
/*! @brief Emits a %item tag without a term. */
void ed_enumerate_item (void);
/*! @brief Emits a %item tag without a term. */
void ed_itemize_item (void);
/*!
 * @brief Emits a %item tag with a term (list environment).
 * @param[in] s Term.  Not NULL.
 */
void ed_list_item (const uchar *s);

/*! @brief Emits the current paragraph (elements[]) with a newline. */
void ed_copy (void);

/*!
 * @brief Emits the opening tag of a verbatim block.
 * @param[in] tag_end  End tag of the block.
 * @param[in] ptmargin Receiver of the current margin (unused).
 */
void ed_verbatim_start (enum tag tag_end, int *ptmargin);

/*!
 * @brief Emits one verbatim line, re-parsed through make_elements.
 * @param[in] tag_end End tag of the block (unused).
 * @param[in] tmargin Current margin (unused).
 * @param[in] compat  Compatibility string (unused).
 */
void ed_verbatim_line (enum tag tag_end, int tmargin, uchar *compat);

/*!
 * @brief Emits the closing tag of a verbatim block.
 * @param[in] tag_end End tag of the block.
 */
void ed_verbatim_end (enum tag tag_end);

/*!
 * @brief Emits %prototype and stashes the compatibility note.
 * @param[in] compat Compatibility string.  Not NULL.
 */
void ed_prototype_start (uchar *compat);

/*!
 * @brief Emits the prototype body, %compat and %endprototype.
 * @param[in] compat Compatibility string.  Not NULL.
 */
void ed_prototype_end (uchar *compat);

/*! @brief Emits a %toc tag. */
void ed_toc_start (void);
/*!
 * @brief Emits one TOC entry (no-op; %toc is emitted by ed_toc_start).
 * @param[in] s  Section number.
 * @param[in] tp TOC node.  Not NULL.
 */
void ed_toc_line (const uchar *s, const struct toc *tp);
/*! @brief Emits the closing of the %toc tag (no-op). */
void ed_toc_end (void);

/*!
 * @brief Emits a %minitoc tag.
 * @param[in] tp Current table-of-contents entry (unused).
 */
void ed_minitoc (const struct toc *tp);

/*!
 * @brief Emits a canonical %function line.
 * @param[in] tp TOC node.  Not NULL.
 */
void ed_function_start (const struct toc *tp);

/*!
 * @brief Per-name hook.  No-op; names are already in tp->title.
 * @param[in] tp TOC node.  Not NULL.
 * @param[in] s  Function name.  Not NULL.
 */
void ed_function_function (const struct toc *tp, const uchar *s);

/*!
 * @brief Emits an index tag.
 * @param[in] tp    TOC entry (unused).
 * @param[in] s     Index entry text.  Not NULL.
 * @param[in] level 0 for %index, 1 for %i1, 2 for %i2.
 */
void ed_index (const struct toc *tp, const uchar *s, int level);

/*! @brief Emits the opening of the see-also list. */
void ed_see_also_start (void);
/*!
 * @brief Emits one see-also reference word.
 * @param[in] word Reference word.  Not NULL.
 * @param[in] s    Remaining list text.  Not NULL.
 */
void ed_see_also_word (const uchar *word, const uchar *s);
/*!
 * @brief Emits the closing of the see-also list.
 * @param[in] s Ignored.
 */
void ed_see_also_end (const uchar *s);

/*!
 * @brief Emits a %samplefile reference.
 * @param[in] s Sample file name.  Not NULL.
 */
void ed_sample_file (const uchar *s);

/*!
 * @brief Emits a libref section heading tag.
 * @param[in] s Section title.  Not NULL.
 */
void ed_libref_section (const uchar *s);

/*!
 * @brief Emits a canonical %table line.
 * @param[in] do_indent Non-zero to indent.
 * @param[in] widths    Column widths.
 * @param[in] wn        Number of columns.
 */
void ed_table_start (int do_indent, int *widths, int wn);

/*!
 * @brief Emits one table row, re-parsed through make_elements.
 * @param[in] s  Row text with 0xb3 cell separators.  Not NULL.
 * @param[in] wn Number of columns (unused).
 */
void ed_table_line (const uchar *s, int wn);

/*! @brief Emits %endtable. */
void ed_table_end (int do_indent);

/*! @brief Emit an HTML fragment anchor (no-op for emxdoc). */
void ed_html_fragment (const uchar *s);
